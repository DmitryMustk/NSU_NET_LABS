#include "../include/client_handler.h"
#include "../include/epoll_wrapper.h"
#include "../include/client_context.h"
#include "../include/logger.h"
#include "../include/socks5.h"

#include <arpa/inet.h>
#include <errno.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 32000

int connectToTargetServer(ClientContext* clientContext, Logger* log) {
    int targetServerFD = socket(AF_INET, SOCK_STREAM, 0);
    if (targetServerFD < 0) {
        logMessage(log, LOG_ERROR, "Failed to create target server socket: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in targetServerAddr;
    memset(&targetServerAddr, 0, sizeof(targetServerAddr));
    targetServerAddr.sin_family = AF_INET;
    targetServerAddr.sin_port = htons(clientContext->serverPort);
    if (inet_pton(AF_INET, clientContext->serverIP, &targetServerAddr.sin_addr) <= 0) {
        logMessage(log, LOG_ERROR, "Invalid target address: %s", strerror(errno));
        close(targetServerFD);
        return -1;
    }

    if (connect(targetServerFD, (struct sockaddr*)&targetServerAddr, sizeof(targetServerAddr)) < 0) {
        logMessage(log, LOG_ERROR, "Failed to connect to target: %s", strerror(errno));
        close(targetServerFD);
        return -1;
    }

    logMessage(log, LOG_INFO, "Connected to target server %s:%d", clientContext->serverIP, clientContext->serverPort);
    return targetServerFD;
}

int forwardTrafficFromClient(ClientContext* clientContext, Logger* log) {
    char buf[BUF_SIZE];
    ssize_t bytesReceived = recv(clientContext->fd, buf, sizeof(buf), 0);
    if (bytesReceived == 0) {
        logMessage(log, LOG_INFO, "Client disconnected");
        return -1;
    }

    logMessage(log, LOG_DEBUG, "Received from client: %zd bytes", bytesReceived);
    ssize_t totalBytesSent = 0;
    while (totalBytesSent < bytesReceived) {
        ssize_t bytesSent = send(clientContext->serverFD, buf + totalBytesSent, bytesReceived - totalBytesSent, 0);
        if (bytesSent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            logMessage(log, LOG_ERROR, "Failed to send data to server: %s", strerror(errno));
            return -1;
        }
        totalBytesSent += bytesSent;
    }
    return 0;
}

int forwardTrafficFromServer(ClientContext* clientContext, Logger* log) {
    char buf[BUF_SIZE];
    ssize_t bytesReceived = recv(clientContext->serverFD, buf, sizeof(buf), 0);
    if (bytesReceived == 0) {
        logMessage(log, LOG_INFO, "Server disconnected");
        return -1;
    }
    logMessage(log, LOG_DEBUG, "Received from server: %zd bytes", bytesReceived);

    ssize_t totalBytesSent = 0;
    while (totalBytesSent < bytesReceived) {
        ssize_t bytesSent = send(clientContext->fd, buf + totalBytesSent, bytesReceived - totalBytesSent, 0);
        if (bytesSent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            logMessage(log, LOG_ERROR, "Failed to send data to client: %s", strerror(errno));
            return -1;
        }
        totalBytesSent += bytesSent;
    }

    return 0;
}

int acceptClient(int serverFD, int epollFD, Logger *log) {
	logMessage(log, LOG_INFO, "Accepting new client...");
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int clientFD = accept(serverFD, (struct sockaddr*)&clientAddr, &addrlen);
	if (clientFD < 0) {
		logMessage(log, LOG_ERROR, "Can't accept client connection");
		return -1;
	}

	char ipStrBuf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr, ipStrBuf, sizeof(ipStrBuf));
	logMessage(log, LOG_INFO, "Accepted new connection: %s", ipStrBuf);

	int flags = fcntl(clientFD, F_GETFL, 0);
	fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);

	ClientContext* clientContext = createClientContext(clientFD);
	if (clientContext == NULL) {
		logMessage(log, LOG_ERROR, "Failed to allocate client context");
		return -1;
	}

	EpollDataWrapper* epollDataWrapper = createEpollDataWrapper(clientContext, CLIENT);
	if (epollDataWrapper == NULL) {
		logMessage(log, LOG_ERROR, "Failed to allocate epoll data wrapper");
		return -1;
	}

    addToEpollSetByPtr(epollFD, clientFD, epollDataWrapper, EPOLLIN | EPOLLET);
	return 0;
}
