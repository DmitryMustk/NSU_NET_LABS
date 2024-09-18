#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 1234
#define PORT_LEN 5

#define BUF_SIZE 256

#define PEEK_IP_ADRESS "8.8.8.8"

//TODO: non blockin io
//TODO: add timer
//TODO: check if there is a need to create mcAdress a second time
//TODO: dynamic output
//TODO: IPv6 support

void autoBindSocket(int sockfd) {
	struct sockaddr_in peekAdress;
	peekAdress.sin_family = AF_INET;
	peekAdress.sin_port = htons(PORT);
	inet_pton(AF_INET, PEEK_IP_ADRESS, &peekAdress.sin_addr);

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

int main(void) {
	const char* mcAdress = "224.0.0.5";

	int readSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (readSocket < 0) {
		perror("Can't open readSocket");
		exit(EXIT_FAILURE);
	}
	
	const int optVal = 1;
	if (setsockopt(readSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0) {
		perror("Can't set REUSEADDR opt to readSocket");
		exit(EXIT_FAILURE);
	}
	setsockopt(readSocket, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal));
	
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	inet_pton(AF_INET, mcAdress, &addr.sin_addr);
	
	if (bind(readSocket, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		perror("Can't bind readSocket");
		exit(EXIT_FAILURE);
	}

	//CONNECT TO MULTICAST GROUP
	struct ip_mreq mreq;
	inet_aton(mcAdress, &(mreq.imr_multiaddr));
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(readSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
		perror("Can't subscribe to MC group on readSocket");
		exit(EXIT_FAILURE);
	}


	int writeSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (writeSocket < 0) {
		perror("Can't open writeSocket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sendAddr;
	bzero(&sendAddr, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, mcAdress, &sendAddr.sin_addr);

	//GET UNIQUE ID OF PROCESS; ID = IP + PORT of writeSocket
	autoBindSocket(writeSocket);
	struct sockaddr_in uniqueAddress;
	socklen_t uniqueAddressLen = sizeof(uniqueAddress);
	if (getsockname(writeSocket, (struct sockaddr*)&uniqueAddress, &uniqueAddressLen) < 0) {
		perror("Can't get name of writeSocket");
		exit(EXIT_FAILURE);
	}
	
	size_t addressBufLen = INET_ADDRSTRLEN + PORT_LEN + 1;
	char adressStrBuf[addressBufLen];
	getAdressStr(uniqueAddress, adressStrBuf, addressBufLen);

	ssize_t responseLen;
	char responseBuf[BUF_SIZE];
	while (1) {
		sendto(writeSocket, adressStrBuf, strlen(adressStrBuf), 0, (struct sockaddr*)&sendAddr, sizeof(addr));
		sleep(1);
		
		responseLen = recvfrom(readSocket, responseBuf, BUF_SIZE, 0, NULL, 0);
		if (strcmp(responseBuf, adressStrBuf) != 0) {
			printf("DETECTED COPY: %s\n", responseBuf);
		} 
		//if (strstr(buf, "UID") && atoi(buf) != uniqueID) {
          //printf("Received message %s", message);

		//}
	}
	close(writeSocket);
	close(readSocket);
	return 0;
}
