#include "../include/socks5.h"
#include "../include/logger.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
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
	uint8_t response[RESPONSE_LEN] = {SOCKS5_VERSION, status, 0x00, ADDR_TYPE_IPV4, 0,0,0,0,0,0};
	return send(clientFD, response, RESPONSE_LEN, 0); 
}

int processSocks5(int clientFD, Logger *log) {
	uint8_t buffer[BUF_SIZE];
    ssize_t bytesReceived = recv(clientFD, buffer, sizeof(buffer), 0);

	if (bytesReceived <= 0) {
		return -1;
	}

	// SOCKS5_HELLO
	if (buffer[0] != SOCKS5_VERSION) {
		sendSocks5Response(clientFD, SOCKS5_GENERAL_FAILURE);
	}

	uint8_t methodCount = buffer[1];
	uint8_t* methods = &buffer[2];
	(void)methods; // NO_AUTH
	

	// Send response
	uint8_t response[2] = {SOCKS5_VERSION, 0x00};
	if (send(clientFD, response, sizeof(response), 0) <= 0) {
		return -1;
	}

	// Get SOCKS5 Request
	bytesReceived = recv(clientFD, buffer, sizeof(buffer), 0);
	if (bytesReceived <= 0 || buffer[1] != CMD_CONNECT) {
		return sendSocks5Response(clientFD, SOCKS5_GENERAL_FAILURE);
	}


	uint8_t addrType = buffer[3];
	char targetAddr[256] = {0};
	uint16_t targetPort;

	if (addrType == ADDR_TYPE_IPV4) {
		inet_ntop(AF_INET, &buffer[4], targetAddr, sizeof(targetAddr));
		targetPort = ntohs(*(uint16_t *)&buffer[8]);
	} else if (addrType == ADDR_TYPE_DOMAIN) {
		uint8_t domainLen = buffer[4];
		memcpy(targetAddr, &buffer[5], domainLen);
		targetPort = ntohs(*(uint16_t *)&buffer[5 + domainLen]);
	} else {
		//IPV6 IS NOT SUPPORTED
		return sendSocks5Response(clientFD, SOCKS5_GENERAL_FAILURE);
	}

	// DNS Resolving
	if (addrType == ADDR_TYPE_DOMAIN) {
		if (resolveDomainNonblocking() < 0) {
			return sendSocks5Response(clientFD, SOCKS5_GENERAL_FAILURE);
		}
		return 0;
	}

	int targetFD = connectToTarget(targetAddr, targetPort);
	if (targetFD < 0) {
		return sendSocks5Response(clientFD, SOCKS5_GENERAL_FAILURE);
	}

	return setupForwarding(clientFD, tartgetFD);
}
