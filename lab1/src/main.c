#include <asm-generic/socket.h>
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

	int uniqueID = rand() % 10000;
	char message[256];
	char buf[256];

	snprintf(message, 256, "%d.UID Msg for multicast pidorast\n", uniqueID);
	
	ssize_t strLen;

	while (1) {
		sendto(writeSocket, message, strlen(message), 0, (struct sockaddr*)&sendAddr, sizeof(addr));
		sleep(1);
		//strLen = recvfrom(readSocket, buf, 256, 0, NULL, 0);

		//if (strstr(buf, "UID") && atoi(buf) != uniqueID) {
          //printf("Received message %s", message);

		//}
	}
	close(writeSocket);
	close(readSocket);
	return 0;
}
