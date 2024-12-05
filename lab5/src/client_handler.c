#include "../include/client_handler.h"
#include "../include/logger.h"

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void acceptClient(int serverFD, int epollFD, Logger *log) {
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int clientFD = accept(serverFD, (struct sockaddr*)&clientAddr, &addrlen);
	if (clientFD < 0) {
		logMessage(log, LOG_ERROR, "Can't accept client connection");
		return;
	}

	int flags = fcntl(clientFD, F_GETFL, 0);
	fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);

	struct epoll_event event = {0};
	event.data.fd = clientFD;
	event.events = EPOLLIN | EPOLLET;

	epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &event);
}

void handleClient(int clientFD, int epollFD, Logger *log) {
	if (processSocks5(clientFD) < 0) {
		close(clientFD);
		epoll_ctl(epollFD, EPOLL_CTL_DEL, clientFD, NULL);
	}
}
