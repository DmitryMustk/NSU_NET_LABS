#include "../include/ipv6_wraps.h"
#include "../include/utils.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT_LEN 5

void configureAddress6(struct sockaddr_in6 *addr, const char *address, in_port_t port) {
	bzero(addr, sizeof(*addr));
	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(port);

	if (address && inet_pton(AF_INET6, address, &addr->sin6_addr) <= 0) {
		handleError("Invalid IPv6 adress");
	}
}

void bindSocket6(int sockfd, const char *address, in_port_t port) {
	struct sockaddr_in6 addr;
	bzero(&addr, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	addr.sin6_addr = in6addr_any;
	printf("Binding to IPv6 address: %s:%d\n", address, ntohs(addr.sin6_port));

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		handleError("Can't bind socket");
	}
}

void joinMCGroup6(int sockfd, const char *address) {
	struct ipv6_mreq mreq;
	if (inet_pton(AF_INET6, address, &mreq.ipv6mr_multiaddr) <= 0) {
		handleError("Invalid mcAdress");
	}
	mreq.ipv6mr_interface = 0;
	if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
		handleError("Can't joinMCGroup6");
	}
}

int getAddressStr6(struct sockaddr* address, char *strBuf, size_t bufSize) {
	if (bufSize < INET6_ADDRSTRLEN + PORT_LEN + 1) {
		return -1;
	}

	struct sockaddr_in6* castedAddr = (struct sockaddr_in6*) address;

	char ipStr[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &castedAddr->sin6_addr, ipStr, sizeof(ipStr));
	snprintf(strBuf, bufSize, "%s:%d", ipStr, htons(castedAddr->sin6_port)); 
	return 0;
}

void autoBindSocket6(int sockfd, const char* peekAddr, in_port_t port) {
	struct sockaddr_in6 peekAddress;
	bzero(&peekAddress, sizeof(peekAddress));

	peekAddress.sin6_family = AF_INET6;
	peekAddress.sin6_port = port;

	if (inet_pton(AF_INET6, peekAddr, &peekAddress.sin6_addr) <= 0) {
		handleError("Invalid peekAddress");
	}

	const char* message = "PEEK_MESSAGE";
	sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&peekAddress, sizeof(peekAddress));
}

