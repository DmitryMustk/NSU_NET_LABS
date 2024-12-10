#include "../include/server.h"
#include "../include/logger.h"
#include "../include/client_handler.h"
#include "../include/socks5.h"

#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EVENTS 64

#define ERROR -1

Logger* logger;

static int setSocketNonblocking(int sockFD) {
	int flags = fcntl(sockFD, F_GETFL, 0);
	if (flags == -1) {
		logMessage(logger, LOG_ERROR, "Can't get flags of sockFD: %s", strerror(errno));
		return -1;
	}

	return fcntl(sockFD, F_SETFL, flags | O_NONBLOCK);
}

int startServer(in_port_t port, Logger *log) {
	logger = log;
	int serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD < 0) {
		logMessage(log, LOG_ERROR, "Failed to create server socket: %s", strerror(errno)); 
		return ERROR;
	}

	int opt = 1;
	if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	    logMessage(log, LOG_ERROR, "setsockopt(SO_REUSEADDR) failed");
	    close(serverFD);
	    return -1;
	}

	if(setSocketNonblocking(serverFD) == ERROR) {
		logMessage(log, LOG_ERROR, "Can't set serverFD nonblocking: %s", strerror(errno));
		return ERROR;
	}

	struct sockaddr_in serverAddr = {0};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = port;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		logMessage(log, LOG_ERROR, "Can't bind server socket: %s", strerror(errno));
		close(serverFD);
		return ERROR;
	}

	if (listen(serverFD, 10) < 0) {
		logMessage(log, LOG_ERROR, "Failed to listen socket: %s", strerror(errno));
		close(serverFD);
		return ERROR;
	} 

	int epollFD = epoll_create1(0);
	if (epollFD < 0) {
		logMessage(log, LOG_ERROR, "Failed to create epoll: %s", strerror(errno));
		close(serverFD);
		return ERROR;
	}

	struct epoll_event event = {0};
	event.data.fd = serverFD;
	event.events = EPOLLIN;
	if (epoll_ctl(epollFD, EPOLL_CTL_ADD, serverFD, &event) < 0) {
		logMessage(log, LOG_ERROR, "Can't add serverFD to epoll event poller: %s", strerror(errno));
		close(serverFD);
		close(epollFD);
		return ERROR;
	}

	struct epoll_event events[MAX_EVENTS];
	while(1) {
		int eventsCount = epoll_wait(epollFD, events, MAX_EVENTS, -1);
		if (eventsCount < 0) {
			logMessage(log, LOG_ERROR, "epoll_wait error: %s", strerror(errno));
			break;
		}	

		for (int i = 0; i < eventsCount; ++i) {
			if (events[i].data.fd == serverFD) {
				logMessage(log, LOG_INFO, "Accepting new client...");
				acceptClient(serverFD, epollFD, log);
			}
			else if (events[i].data.ptr != NULL) {
				logMessage(log, LOG_DEBUG, "New client event detected");
				EpollDataWrapper* wrapper = events[i].data.ptr;
				if (wrapper->clientContextPtr->state != STATE_FORWARDING) {
					handleClientState(wrapper->clientContextPtr, epollFD, log);
				} else if (wrapper->type == CLIENT) {
					logMessage(log, LOG_DEBUG, "Start forwarding data from client");
					forwardTrafficFromClient(wrapper->clientContextPtr, log);
				} else if (wrapper->type == TARGET_SERVER) {
					forwardTrafficFromServer(wrapper->clientContextPtr, log);
				}
			} 
		}
	}

	close(serverFD);
	close(epollFD);

	return 0;
}
