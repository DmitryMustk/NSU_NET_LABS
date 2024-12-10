#include "../include/socks5.h"
#include "../include/logger.h"
#include "../include/client_context.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#define SOCKS5_VERSION         0x05
#define CMD_CONNECT            0x01
#define ADDR_TYPE_IPV4         0x01
#define ADDR_TYPE_DOMAIN       0x03
#define SOCKS5_SUCCESS         0x00
#define SOCKS5_GENERAL_FAILURE 0x01

#define RESPONSE_LEN 10
#define BUF_SIZE 16384

#define MAX_EVENTS 256

static int sendSocks5Response(int clientFD, uint8_t status) {
    uint8_t response[RESPONSE_LEN] = {SOCKS5_VERSION, status, 0x00, ADDR_TYPE_IPV4, 0, 0, 0, 0, 0, 0};
    return send(clientFD, response, RESPONSE_LEN, 0); 
}

static int sendSocks5ConnectResponse(ClientContext* clientContext, Logger* log) {
    uint8_t response[10] = {SOCKS5_VERSION, SOCKS5_SUCCESS, 0x00, ADDR_TYPE_IPV4};
    struct sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);

    if (getsockname(clientContext->serverFD, (struct sockaddr*)&localAddr, &localAddrLen) < 0) {
        logMessage(log, LOG_ERROR, "Failed to retrieve socket address: %s", strerror(errno));
        sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
        close(clientContext->serverFD);
        return -1;
    }

    memcpy(&response[4], &localAddr.sin_addr, 4);  // BND.ADDR (IPv4, 4 bytes)
    memcpy(&response[8], &localAddr.sin_port, 2); // BND.PORT (2 bytes, network order)

    if (send(clientContext->fd, response, sizeof(response), 0) < 0) {
        logMessage(log, LOG_ERROR, "Failed to send SOCKS5 response: %s", strerror(errno));
        close(clientContext->serverFD);
        return -1;
    }
    
    return 0;
}

static int processHelloRequest(ClientContext* clientContext, Logger* log) {
    uint8_t buffer[BUF_SIZE];
    ssize_t bytesReceived = recv(clientContext->fd, buffer, sizeof(buffer), 0);
	if (bytesReceived <= 0) {
        return -1;
    }
	// +----+----------+----------+
    // |VER | NMETHODS | METHODS  |
    // +----+----------+----------+
    // | 1  |    1     | 1 to 255 |
    // +----+----------+----------+
	logHexMessage(log, LOG_DEBUG, buffer, bytesReceived);
        
    if (buffer[0] != SOCKS5_VERSION) {
        sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
        return -1;
    }

	//TODO: NO_AUTH verification
    uint8_t methodCount = buffer[1];
    uint8_t* methods = &buffer[2];
    (void)methods; // NO_AUTH

    // Send response
	//  +----+--------+
    //  |VER | METHOD |
    //  +----+--------+
    //  | 1  |   1    |
    //  +----+--------+
	// o  X'00' NO AUTHENTICATION REQUIRED
    // o  X'01' GSSAPI
    // o  X'02' USERNAME/PASSWORD
    // o  X'03' to X'7F' IANA ASSIGNED
    // o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
    // o  X'FF' NO ACCEPTABLE METHODS
    uint8_t response[2] = {SOCKS5_VERSION, 0x00};
    if (send(clientContext->fd, response, sizeof(response), 0) <= 0) {
        return -1;
    }

    clientContext->state = STATE_HELLO;
    return 0;
}

static int processConnectionRequest(ClientContext* clientContext, Logger* log) {
    uint8_t buffer[BUF_SIZE];
    ssize_t bytesReceived = recv(clientContext->fd, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0 || buffer[1] != CMD_CONNECT) {
        return sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
    }
    
	logHexMessage(log, LOG_DEBUG, buffer, bytesReceived);

    uint8_t addrType = buffer[3];
    char targetAddr[256] = {0};
    uint16_t targetPort;

    if (addrType == ADDR_TYPE_IPV4) {
        inet_ntop(AF_INET, &buffer[4], targetAddr, sizeof(targetAddr));
        targetPort = ntohs(*(uint16_t *)&buffer[8]);
    } else if (addrType == ADDR_TYPE_DOMAIN) {
        // TODO: add domain addrType support
        uint8_t domainLen = buffer[4];
        memcpy(targetAddr, &buffer[5], domainLen);
        targetAddr[domainLen] = '\0';
        targetPort = ntohs(*(uint16_t *)&buffer[5 + domainLen]);
    } else {
        // IPv6 is not supported
        return sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
    }

    logMessage(log, LOG_DEBUG, "Target addr: %s:%d", targetAddr, targetPort);
    clientContext->state = STATE_CONNECTION;

    int targetServerFD = socket(AF_INET, SOCK_STREAM, 0);
    if (targetServerFD < 0) {
        logMessage(log, LOG_ERROR, "Failed to create target server socket: %s", strerror(errno));
        sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
        return -1;
    }

    struct sockaddr_in targetServerAddr;
    memset(&targetServerAddr, 0, sizeof(targetServerAddr));
    targetServerAddr.sin_family = AF_INET;
    targetServerAddr.sin_port = htons(targetPort);
    if (inet_pton(AF_INET, targetAddr, &targetServerAddr.sin_addr) <= 0) {
        logMessage(log, LOG_ERROR, "Invalid target address: %s", strerror(errno));
        sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
        close(targetServerFD);
        return -1;
    }

    if (connect(targetServerFD, (struct sockaddr*)&targetServerAddr, sizeof(targetServerAddr)) < 0) {
        logMessage(log, LOG_ERROR, "Failed to connect to target: %s", strerror(errno));
        sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
        close(targetServerFD);
        return -1;
    } 

    logMessage(log, LOG_INFO, "Connected to target server %s:d", targetAddr, targetPort);


    clientContext->serverFD = targetServerFD;
    clientContext->state = STATE_FORWARDING;
    sendSocks5ConnectResponse(clientContext, log);
    return 0;
}

static int forwardTrafficFromClient(ClientContext* clientContext, Logger* log) {
    char buf[BUF_SIZE];
    ssize_t bytesReceived = recv(clientContext->fd, buf, sizeof(buf), 0);
    if (bytesReceived == 0) {
        logMessage(log, LOG_INFO, "Client disconnected");
        return -1;
    }

    logMessage(log, LOG_DEBUG, "Received from client: %zd bytes", bytesReceived);
    ssize_t totalBytesSent = 0;
    while (totalBytesSent < bytesReceived) {
        ssize_t bytesSent = send(clientContext->serverFD, buf + totalBytesSent, bytesReceived - totalBytesSent, 0);
        if (bytesSent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            logMessage(log, LOG_ERROR, "Failed to send data to server: %s", strerror(errno));
            return -1;
        }
        totalBytesSent += bytesSent;
    }
    return 0;
}

static int forwardTrafficFromServer(ClientContext* clientContext, Logger* log) {
    char buf[BUF_SIZE];
    ssize_t bytesReceived = recv(clientContext->serverFD, buf, sizeof(buf), 0);
    if (bytesReceived == 0) {
        logMessage(log, LOG_INFO, "Server disconnected");
        return -1;
    }
    logMessage(log, LOG_DEBUG, "Received from server: %zd bytes", bytesReceived);

    ssize_t totalBytesSent = 0;
    while (totalBytesSent < bytesReceived) {
        ssize_t bytesSent = send(clientContext->fd, buf + totalBytesSent, bytesReceived - totalBytesSent, 0);
        if (bytesSent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            logMessage(log, LOG_ERROR, "Failed to send data to client: %s", strerror(errno));
            return -1;
        }
        totalBytesSent += bytesSent;
    }

    return 0;
}

static int handleClientState(ClientContext* clientContext, Logger* log) {
    if (clientContext->state == STATE_NONE) {
        return processHelloRequest(clientContext, log);
    } else if (clientContext->state == STATE_HELLO) {
        return processConnectionRequest(clientContext, log);
    } else if (clientContext->state == STATE_FORWARDING) {
        return forwardTrafficFromClient(clientContext, log);
    }

    logMessage(log, LOG_ERROR, "Unknown state: %d", clientContext->state);
    return -1;
}

int processSocks5(ClientContext* clientContext, Logger *log) {
    int epollFD = epoll_create1(0);
    if (epollFD == -1) {
        logMessage(log, LOG_ERROR, "Failed to create epoll instance: %s", strerror(errno));
        return -1;
    }

    // Add client socket as a epoll event to set
    struct epoll_event epollEvent;
    epollEvent.events = EPOLLIN | EPOLLET; // read with edge-trigerred handling
    epollEvent.data.fd = clientContext->fd;
    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientContext->fd, &epollEvent) == -1) {
        logMessage(log, LOG_ERROR, "Failed to register client fd with epoll: %s", strerror(errno));
        close(epollFD);
        return -1;
    }

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int n = epoll_wait(epollFD, events, MAX_EVENTS, -1);
        if (n == -1) {
            if (errno == EINTR) {
                // syscall been interrupted, try again
                continue;
            }
            logMessage(log, LOG_ERROR, "epoll_wait failed: %s", strerror(errno));
            close(epollFD);
            return -1;
        }

        for (int i = 0; i < n; ++i) {
            if (events[i].events & EPOLLIN)  {
                
                if (events[i].data.fd == clientContext->fd) {
                    int res = handleClientState(clientContext, log);
                    if (res == -1) {
                        close(epollFD);
                        return -1;
                    }

                    // Add server sock to epoll set
                    if (clientContext->state == STATE_FORWARDING && !clientContext->isServerFDPolling) {
                        epollEvent.data.fd = clientContext->serverFD;
                        if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientContext->serverFD, &epollEvent) == -1) {
                            logMessage(log, LOG_ERROR, "Failed to register server socket in epoll set: %s", strerror(errno));
                            close(epollFD);
                            return -1;
                        }
                        clientContext->isServerFDPolling = 1;
                    }
                }

                if (events[i].data.fd == clientContext->serverFD) {
                    if (clientContext->state == STATE_FORWARDING) {
                        if (forwardTrafficFromServer(clientContext, log) == -1) {
                            close(epollFD);
                            return -1;
                        }
                    } else {
                        logMessage(log, LOG_ERROR, "ClientContext has serverFD, but he isn't on state forwarding, wtf?");
                        close(epollFD);
                        return -1;
                    }
                }
            }
        }
    }
    


    return 0;
}
