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
#define BUF_SIZE     512

static int sendSocks5Response(int clientFD, uint8_t status) {
    uint8_t response[RESPONSE_LEN] = {SOCKS5_VERSION, status, 0x00, ADDR_TYPE_IPV4, 0, 0, 0, 0, 0, 0};
    return send(clientFD, response, RESPONSE_LEN, 0); 
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
        uint8_t domainLen = buffer[4];
        memcpy(targetAddr, &buffer[5], domainLen);
        targetAddr[domainLen] = '\0';
        targetPort = ntohs(*(uint16_t *)&buffer[5 + domainLen]);
    } else {
        // IPv6 is not supported
        return sendSocks5Response(clientContext->fd, SOCKS5_GENERAL_FAILURE);
    }

    logMessage(log, LOG_DEBUG, "Target addr: %s:%d", targetAddr, targetPort);
    return 0;
}

int processSocks5(ClientContext* clientContext, Logger *log) {
    if (clientContext->state == STATE_NONE) {
        return processHelloRequest(clientContext, log);
    }
    else if (clientContext->state == STATE_HELLO) {
        return processConnectionRequest(clientContext, log);
    }
    return 0;

}
