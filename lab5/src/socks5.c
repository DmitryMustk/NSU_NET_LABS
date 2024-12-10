#include "../include/socks5.h"
#include "../include/logger.h"
#include "../include/client_context.h"
#include "../include/server.h"

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

#define MAX_EVENTS 64

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
    // uint8_t methodCount = buffer[1];
    // uint8_t* methods = &buffer[2];

    // Send response           o  X'00' NO AUTHENTICATION REQUIRED
	//  +----+--------+        o  X'01' GSSAPI
    //  |VER | METHOD |        o  X'02' USERNAME/PASSWORD
    //  +----+--------+        o  X'03' to X'7F' IANA ASSIGNED
    //  | 1  |   1    |        o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
    //  +----+--------+        o  X'FF' NO ACCEPTABLE METHODS
    uint8_t response[2] = {SOCKS5_VERSION, 0x00};
    if (send(clientContext->fd, response, sizeof(response), 0) <= 0) {
        return -1;
    }

    clientContext->state = STATE_HELLO;
    return 0;
}


static int receiveAndValidateRequest(ClientContext* clientContext, Logger* log, uint8_t* buffer, size_t bufferSize) {
    ssize_t bytesReceived = recv(clientContext->fd, buffer, bufferSize, 0);
    if (bytesReceived <= 0 || buffer[1] != CMD_CONNECT) {
        return sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
    }
    logHexMessage(log, LOG_DEBUG, buffer, bytesReceived);
    return 0;
}

static int extractTargetAddressAndPort(const uint8_t* buffer, char* targetAddr, size_t addrSize, uint16_t* targetPort, Logger* log) {
    uint8_t addrType = buffer[3];

    if (addrType == ADDR_TYPE_IPV4) {
        inet_ntop(AF_INET, &buffer[4], targetAddr, addrSize);
        *targetPort = ntohs(*(uint16_t*)&buffer[8]);
    } else if (addrType == ADDR_TYPE_DOMAIN) {
        logMessage(log, LOG_ERROR, "Domain addr type is not supported yet");
        // uint8_t domainLen = buffer[4];
        // memcpy(targetAddr, &buffer[5], domainLen);
        // targetAddr[domainLen] = '\0';
        // *targetPort = ntohs(*(uint16_t*)&buffer[5 + domainLen]);
    } else {
        logMessage(log, LOG_ERROR, "IPv6 is not supported");
        return -1;
    }

    logMessage(log, LOG_DEBUG, "Target addr: %s:%d", targetAddr, *targetPort);
    return 0;
}

static int createAndConnectToTarget(char* targetAddr, uint16_t targetPort, Logger* log) {
    int targetServerFD = socket(AF_INET, SOCK_STREAM, 0);
    if (targetServerFD < 0) {
        logMessage(log, LOG_ERROR, "Failed to create target server socket: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in targetServerAddr;
    memset(&targetServerAddr, 0, sizeof(targetServerAddr));
    targetServerAddr.sin_family = AF_INET;
    targetServerAddr.sin_port = htons(targetPort);
    if (inet_pton(AF_INET, targetAddr, &targetServerAddr.sin_addr) <= 0) {
        logMessage(log, LOG_ERROR, "Invalid target address: %s", strerror(errno));
        close(targetServerFD);
        return -1;
    }

    if (connect(targetServerFD, (struct sockaddr*)&targetServerAddr, sizeof(targetServerAddr)) < 0) {
        logMessage(log, LOG_ERROR, "Failed to connect to target: %s", strerror(errno));
        close(targetServerFD);
        return -1;
    }

    logMessage(log, LOG_INFO, "Connected to target server %s:%d", targetAddr, targetPort);
    return targetServerFD;
}

static int handleConnectionFailure(int clientFD, int serverFD, Logger* log) {
    sendSocks5Response(clientFD, SOCKS5_GENERAL_FAILURE);
    if (serverFD >= 0) {
        logMessage(log, LOG_ERROR, "Connection failured");
        close(serverFD);
    }
    return -1;
}

static int processConnectionRequest(ClientContext* clientContext, Logger* log) {
    uint8_t buffer[BUF_SIZE];
    if (receiveAndValidateRequest(clientContext, log, buffer, sizeof(buffer)) < 0) {
        return -1;
    }

    char targetAddr[256] = {0};
    uint16_t targetPort;
    if (extractTargetAddressAndPort(buffer, targetAddr, sizeof(targetAddr), &targetPort, log) < 0) {
        return sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
    }

    int targetServerFD = createAndConnectToTarget(targetAddr, targetPort, log);
    if (targetServerFD < 0) {
        return handleConnectionFailure(clientContext->fd, targetServerFD, log);
    }

    clientContext->serverFD = targetServerFD;
    clientContext->state = STATE_FORWARDING;

    sendSocks5ConnectResponse(clientContext, log);
    return 0;
}

int handleClientState(ClientContext* clientContext, int epollFD, Logger* log) {
    if (clientContext->state == STATE_NONE) {
        return processHelloRequest(clientContext, log);
    } else if (clientContext->state == STATE_HELLO) {
        if (processConnectionRequest(clientContext, log) == -1) {
            return -1;
        }
        
        EpollDataWrapper* wrap = malloc(sizeof(EpollDataWrapper));
        wrap->type = TARGET_SERVER;
        wrap->clientContextPtr = clientContext;

        struct epoll_event targetServerEvent;
        targetServerEvent.events = EPOLLIN;
        targetServerEvent.data.ptr = wrap;

        if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientContext->serverFD, &targetServerEvent) == -1) {
            logMessage(log, LOG_ERROR, "Failed to register target server fd with epoll: %s", strerror(errno));
            close(epollFD);
            return -1;
        }
        return 0;
    } 
    logMessage(log, LOG_ERROR, "Unknown state: %d", clientContext->state);
    return -1;
}
