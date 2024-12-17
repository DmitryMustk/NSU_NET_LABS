#include "../include/socks5.h"
#include "../include/logger.h"
#include "../include/client_handler.h"
#include "../include/client_context.h"
#include "../include/dns_resolver.h"
#include "../include/server.h"
#include "../include/epoll_wrapper.h"

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
#define SOCKS5_CONNECTION_NOT_ALLOWED_BY_RULESET 0x02

#define DOMAIN_MAX_LEN 256 + 1
#define RESPONSE_LEN 10
#define BUF_SIZE 16384

#define MAX_EVENTS 64

static int sendSoextractTargetAddressAndPortcks5Response(int clientFD, uint8_t status) {
    uint8_t response[RESPONSE_LEN] = {SOCKS5_VERSION, status, 0x00, ADDR_TYPE_IPV4, 0, 0, 0, 0, 0, 0};
    return send(clientFD, response, RESPONSE_LEN, 0); 
}

static int sendSocks5ConnectResponse(ClientContext* clientContext, uint8_t reply, Logger* log) {
    uint8_t response[10] = {SOCKS5_VERSION, reply, 0x00, ADDR_TYPE_IPV4};
    struct sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);

    if (getsockname(clientContext->serverFD, (struct sockaddr*)&localAddr, &localAddrLen) < 0) {
        logMessage(log, LOG_ERROR, "Failed to retrieve socket address: %s", strerror(errno));
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
    logMessage(log, LOG_DEBUG, "Processing hello request");
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
        return -1;
    }
    logHexMessage(log, LOG_DEBUG, buffer, bytesReceived);
    return 0;
}

static int extractTargetAddressAndPort(ClientContext* clientContext, const uint8_t* buffer, char* domain, Logger* log) {
    uint8_t addrType = buffer[3];

    if (addrType == ADDR_TYPE_IPV4) {
        clientContext->addrType = IPV4;
        inet_ntop(AF_INET, &buffer[4], clientContext->serverIP, INET_ADDRSTRLEN);
        clientContext->serverPort = ntohs(*(uint16_t*)&buffer[8]);
        clientContext->isDomainResolved = 1;
    } else if (addrType == ADDR_TYPE_DOMAIN) {
        clientContext->addrType = IPV6;
        uint8_t domainLen = buffer[4];
        memcpy(domain, &buffer[5], domainLen);
        domain[domainLen] = '\0';
        clientContext->serverPort = ntohs(*(uint16_t*)&buffer[5 + domainLen]);
        clientContext->isDomainResolved = 0;
    } else {
        clientContext->addrType = IPV6;
        logMessage(log, LOG_ERROR, "IPv6 is not supported");
        sendSocks5ConnectResponse(clientContext, SOCKS5_CONNECTION_NOT_ALLOWED_BY_RULESET, log);
        return -1;
    }

    return 0;
}

static int handleConnectionFailure(int clientFD, int serverFD, Logger* log) {
    if (serverFD >= 0) {
        logMessage(log, LOG_ERROR, "Connection failured");
        close(serverFD);
    }
    return -1;
}

static int processConnectionRequest(ClientContext* clientContext, DnsResolver* dnsResolver, Logger* log) {
    logMessage(log, LOG_DEBUG, "Processing connection request");
    uint8_t buffer[BUF_SIZE];
    if (receiveAndValidateRequest(clientContext, log, buffer, sizeof(buffer)) < 0) {
        return -1;
    }
    logMessage(log, LOG_DEBUG, "Request received");

    char domain[DOMAIN_MAX_LEN] = {0};
    logMessage(log, LOG_DEBUG, "Start extracting adress");
    if (extractTargetAddressAndPort(clientContext, buffer, domain, log) < 0) {
        return -1;
    }
    logMessage(log, LOG_DEBUG, "Send dns request");

    if (!clientContext->isDomainResolved) {
        return sendDnsRequest(dnsResolver, clientContext, domain, log);
    }
    logMessage(log, LOG_DEBUG, "Connect to target server");

    int targetServerFD = connectToTargetServer(clientContext, log);
    logMessage(log, LOG_DEBUG, "Connected to target server");
    if (targetServerFD < 0) {
        return handleConnectionFailure(clientContext->fd, targetServerFD, log);
    }

    clientContext->serverFD = targetServerFD;
    clientContext->state = STATE_FORWARDING;

    logMessage(log, LOG_DEBUG, "Send connect response");
    sendSocks5ConnectResponse(clientContext, SOCKS5_SUCCESS, log);
    logMessage(log, LOG_DEBUG, "Connection request been processed");
    return 0;
}

int handleClientState(ClientContext* clientContext, int epollFD, DnsResolver* dnsResolver, Logger* log) {
    if (clientContext->state == STATE_NONE) {
        return processHelloRequest(clientContext, log);
    } else if (clientContext->state == STATE_HELLO) {
        if (processConnectionRequest(clientContext, dnsResolver, log) == -1) {
            return -1;
        }
        return addServerFDToEpollSet(epollFD, clientContext);;
    } 
    logMessage(log, LOG_ERROR, "Unknown state: %d", clientContext->state);
    return -1;
}
