#include "../include/client_handler.h"
#include "../include/client_context.h"
#include "../include/logger.h"
#include "../include/socks5.h"

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

	char* ipStrBuf = malloc(sizeof(char) * INET_ADDRSTRLEN);
	logMessage(log, LOG_INFO, "Accepted new connection: %s", inet_ntop(AF_INET, &clientAddr, ipStrBuf, addrlen));
	free(ipStrBuf);

	int flags = fcntl(clientFD, F_GETFL, 0);
	fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);

	ClientContext* clientContext = createClientContext(clientFD);
	if (!clientContext) {
		logMessage(log, LOG_ERROR, "Failed to allocate client context");
		close(clientFD);
		return;
	}

	struct epoll_event event = {0};
	event.events = EPOLLIN | EPOLLET;
	event.data.ptr = clientContext;

	epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &event);
}

void handleClient(ClientContext* clientContext, int epollFD, Logger *log) {
	if (processSocks5(clientContext, log) < 0) {
		freeClientContext(clientContext);
		epoll_ctl(epollFD, EPOLL_CTL_DEL, clientContext->fd, NULL);
	}
}
