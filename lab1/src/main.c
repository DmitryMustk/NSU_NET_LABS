#include <asm-generic/socket.h>
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

#define PORT 1234
#define PORT_LEN 5

#define BUF_SIZE 256

#define PEEK_IP_ADRESS "8.8.8.8"

//TODO: add timer
//TODO: check if there is a need to create mcAdress a second time
//TODO: dynamic output
//TODO: IPv6 support
//TODO: user input address support

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

void bindSocket(int sockfd, const char* address, in_port_t port) {
	struct sockaddr_in addr;

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, address, &addr.sin_addr) <= 0) {
		handleError("Invalid adress");
	}
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		handleError("Can't bind socket");
	}
}

void autoBindSocket(int sockfd) {
	struct sockaddr_in peekAdress;
	peekAdress.sin_family = AF_INET;
	peekAdress.sin_port = htons(PORT);
	if (inet_pton(AF_INET, PEEK_IP_ADRESS, &peekAdress.sin_addr) <= 0) {
		handleError("Invalid peekAdress");
	}

	const char* message = "PEEK_MESSAGE";
	sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&peekAdress, sizeof(peekAdress));
}

int getAdressStr(struct sockaddr_in address, char* strBuf, size_t bufSize) {
	//2 = ':' + '\0'
	if (bufSize < INET_ADDRSTRLEN + PORT_LEN + 1) {
		printf("faf");
		return -1;
	}

	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &address.sin_addr, ipStr, sizeof(ipStr));
	snprintf(strBuf, bufSize, "%s:%d", ipStr, ntohs(address.sin_port));
//	printf("ADRESS %s", strBuf);
	return 0;
}

void joinMCGroup(int sockfd, const char* adress) {
	struct ip_mreq mreq;
	if (inet_aton(adress, &mreq.imr_multiaddr) == 0) {
		handleError("Invalid mcAdress");
	}
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
		handleError("Can't join mcGroup");
	}
}

void getSocketName(int sockfd, struct sockaddr_in* addr, socklen_t* addrSize) {
	if (getsockname(sockfd, (struct sockaddr*)addr, addrSize) < 0) {
		handleError("Can't get name of socket");
	}
} 

int main(void) {
	const char* mcAdress = "224.0.0.5";

	int readSocket = createSocket(AF_INET, SOCK_DGRAM, 0); 
	setSocketReuse(readSocket);	
	bindSocket(readSocket, mcAdress, PORT);	

	joinMCGroup(readSocket, mcAdress);	


	int writeSocket = createSocket(AF_INET, SOCK_DGRAM, 0); 

	struct sockaddr_in sendAddr;
	bzero(&sendAddr, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, mcAdress, &sendAddr.sin_addr) <= 0) {
		handleError("Invalid send adress");
	}

	//GET UNIQUE ID OF PROCESS; ID = IP + PORT of writeSocket
	autoBindSocket(writeSocket);

	struct sockaddr_in uniqueAddress;
	socklen_t uniqueAddressLen = sizeof(uniqueAddress);
	getSocketName(writeSocket, &uniqueAddress, &uniqueAddressLen);
		
	size_t addressBufLen = INET_ADDRSTRLEN + PORT_LEN + 1;
	char adressStrBuf[addressBufLen];
	getAdressStr(uniqueAddress, adressStrBuf, addressBufLen);

	ssize_t responseLen;
	char responseBuf[BUF_SIZE];

	//Timeout for select
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	fd_set readFds;

	while (1) {
		FD_ZERO(&readFds);
		FD_SET(readSocket, &readFds);

		int activity = select(readSocket + 1, &readFds, NULL, NULL, &timeout);
		if (activity < 0) {
			handleError("select error");
		} else if (activity != 0) {
			if (FD_ISSET(readSocket, &readFds)) {
				responseLen = recvfrom(readSocket, responseBuf, BUF_SIZE, 0, NULL, NULL);
				if (responseLen < 0) {
					handleError("Failed to receive message");
				}
				if (strcmp(responseBuf, adressStrBuf) != 0) {
					printf("DETECTED COPY: %s\n", responseBuf);
				} 

			}
		}

		sendto(writeSocket, adressStrBuf, strlen(adressStrBuf), 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
		sleep(1);
		
	}
	close(writeSocket);
	close(readSocket);
	return 0;
}
