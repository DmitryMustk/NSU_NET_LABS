#include "../include/ipv4_wraps.h"
#include "../include/utils.h" 

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT_LEN 5

void configureAddress(struct sockaddr_in* addr, const char* address, in_port_t port) {
    bzero(addr, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    if (address && inet_pton(AF_INET, address, &addr->sin_addr) <= 0) {
        handleError("Invalid IPv4 address");
    }
}

void bindSocket(int sockfd, in_port_t port) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		handleError("Can't bind socket");
    }
}

void joinMCGroup(int sockfd, const char* address) {
    struct ip_mreq mreq;
    if (inet_aton(address, &mreq.imr_multiaddr) == 0) {
        handleError("Invalid mcAddress");
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
        handleError("Can't join mcGroup");
    }
}

int getAddressStr(struct sockaddr* address, char* strBuf, size_t bufSize) {
    if (bufSize < INET_ADDRSTRLEN + PORT_LEN + 1) {
        return -1;
    }

	struct sockaddr_in* castedAddr = (struct sockaddr_in*)address;

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &castedAddr->sin_addr, ipStr, sizeof(ipStr));
    snprintf(strBuf, bufSize, "%s:%d", ipStr, ntohs(castedAddr->sin_port));
    return 0;
}

void autoBindSocket(int sockfd, const char* peekAddr, in_port_t peekPort) {
	struct sockaddr_in peekAddress;
	peekAddress.sin_family = AF_INET;
	peekAddress.sin_port = htons(peekPort);
	if (inet_pton(AF_INET, peekAddr, &peekAddress.sin_addr) <= 0) {
		handleError("Invalid peekAddress");
	}

	const char* message = "PEEK_MESSAGE";
	sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&peekAddress, sizeof(peekAddress));
}

