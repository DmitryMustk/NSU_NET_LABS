#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../include/utils.h"
#include "../include/timer.h"
#include "../include/copies_set.h"

#include "../include/ipv6_wraps.h"
#include "../include/ipv4_wraps.h"

#define PORT 1234
#define PORT_LEN 5
#define BUF_SIZE 256
#define PEEK_IP_ADRESS  "8.8.8.8"
#define PEEK_IP6_ADRESS "::ffff:8.8.8.8"

#define MAX_COPIES 128

#define COPY_TTL 3

//TODO: IPv6 support
//TODO: dynamic lib for ipv4 and ipv6


void sendAndRecvMessages(int writeSocket, int readSocket, const struct sockaddr* sendAddr, const char* addressStr);

int createSocket(int domain, int type, int protocol) {
	int sockfd = socket(domain, type, protocol);
	if (sockfd < 0) {
		handleError("Can't open socket");
	}

	return sockfd;
}

void setSocketReuse(int sockfd) {
	const int optVal = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0 ||
			setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal)) != 0) {
		handleError("Can't set socket options");
	}
}

void getSocketName(int sockfd, struct sockaddr* addr, socklen_t* addrSize) {
	if (getsockname(sockfd, addr, addrSize) < 0) {
		handleError("Can't get name of socket");
	}
} 

void handleSocketCommunication(int readSocket, int writeSocket, const char* mcAddress, const char* uniqueAddressStr, int isIPv6) {
	if (isIPv6) {
		struct sockaddr_in6 sendAddr;
		configureAddress6(&sendAddr, mcAddress, PORT);
		sendAndRecvMessages(writeSocket, readSocket, (struct sockaddr*)&sendAddr, uniqueAddressStr);
		return;
	}
	struct sockaddr_in sendAddr;
	configureAddress(&sendAddr, mcAddress, PORT);
	sendAndRecvMessages(writeSocket, readSocket, (struct sockaddr*)&sendAddr, uniqueAddressStr);
}

void sendAndRecvMessages(int writeSocket, int readSocket, const struct sockaddr* sendAddr, const char* addressStr) {
	ssize_t responseLen;
	char responseBuf[BUF_SIZE];

	fd_set readFds;
	
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	Timer timer;
	startTimer(&timer, 2);

	CopiesSet copiesSet = {0, MAX_COPIES, {0}};

	while (1) {
		FD_ZERO(&readFds);
		FD_SET(readSocket, &readFds);

		int activity = select(readSocket + 1, &readFds, NULL, NULL, &timeout);
		if (activity < 0) {
			handleError("select error");
		} else if (activity != 0 && FD_ISSET(readSocket, &readFds)) {
			responseLen = recvfrom(readSocket, responseBuf, BUF_SIZE, 0, NULL, NULL);
			responseBuf[responseLen] = '\0';
			if (responseLen < 0) {
				handleError("Failed to receive message");
			}
			//printf("%s\n", responseBuf);
			if (strcmp(responseBuf, addressStr) != 0) {
				Copy newCopy = {{0}, time(NULL)};
				memcpy(newCopy.name, responseBuf, responseLen);

				appendToCopiesSet(&copiesSet, &newCopy);
			} 
		}
		
		if (timerExpired(&timer)) {
			printCopiesSet(&copiesSet);
			sendto(writeSocket, addressStr, strlen(addressStr), 0, sendAddr, sizeof(*sendAddr));

			removeDeadCopies(&copiesSet);
			resetTimer(&timer);
		}

	}
}


void initSocket(int* sockfd, const char* address, in_port_t port, int isIPv6) {
	if (isIPv6) {
		*sockfd = createSocket(AF_INET6, SOCK_DGRAM, 0);
	} else {
		*sockfd = createSocket(AF_INET, SOCK_DGRAM, 0);
	}
	setSocketReuse(*sockfd);
	
	if (address == NULL) {
		return;
	}

	if (isIPv6) {
		//printf("JJJJJJ\n");
		//bindSocket6(*sockfd, "::1", port);
		bindSocket6(*sockfd, address, port);
		return;
	}
	bindSocket(*sockfd, address, port);
}


int main(int argc, char** argv) {
	//const char* mcAddress = "224.0.0.5";
	int readSocket, writeSocket;
	
	if (argc < 2) {
		printf("Usage: $%s <IPv4/6 Multicast adress>\n", argv[0]);
		return 0;
	}
	const char* mcAddress = argv[1];
	int isIPv6 = (strchr(mcAddress, ':') != NULL);
	//printf("IPV6 %d\n", isIPv6);
//	int isIPv6 = 0;

	initSocket(&readSocket, mcAddress, PORT, isIPv6);
	if (isIPv6) {
		joinMCGroup6(readSocket, mcAddress);
	} else {
		joinMCGroup(readSocket, mcAddress);
	}

	initSocket(&writeSocket, NULL, 0, isIPv6);
	
	if (isIPv6) {
		autoBindSocket6(writeSocket, PEEK_IP6_ADRESS, PORT);
	}
	else {
		autoBindSocket(writeSocket, PEEK_IP_ADRESS, PORT);
	}


	struct sockaddr uniqueAddress;
	socklen_t uniqueAddressLen = sizeof(uniqueAddress);
	getSocketName(writeSocket, (struct sockaddr*)&uniqueAddress, &uniqueAddressLen);

	char addressStr[INET6_ADDRSTRLEN + PORT_LEN + 1];
	if (isIPv6) {
		getAddressStr6(&uniqueAddress, addressStr, sizeof(addressStr));
	} else {
		getAddressStr(&uniqueAddress, addressStr, sizeof(addressStr));
	}

	printf("MY ADDRESS: %s\n", addressStr);
	handleSocketCommunication(readSocket, writeSocket, mcAddress, addressStr, isIPv6);

		
	close(writeSocket);
	close(readSocket);
	return 0;
}
