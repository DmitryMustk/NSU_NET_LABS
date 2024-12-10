#include "../include/server.h"
#include "../include/logger.h"
#include "../include/client_handler.h"
#include "../include/epoll_wrapper.h"
#include "../include/socks5.h"

#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_EVENTS 64

static int setSocketNonblocking(int sockFD, Logger* log) {
	int flags = fcntl(sockFD, F_GETFL, 0);
	if (flags == -1) {
		logMessage(log, LOG_ERROR, "Can't get flags of sockFD: %s", strerror(errno));
		return -1;
	}

	return fcntl(sockFD, F_SETFL, flags | O_NONBLOCK);
}

static int startPolling(int serverFD, Logger* log) {
	int epollFD = createEpollInstance(log);
	if (addToEpollSetByFD(epollFD, serverFD, EPOLLIN) < 0) {
		logMessage(log, LOG_ERROR, "Can't add serverFD to epoll event poller: %s", strerror(errno));
		close(serverFD);
		close(epollFD);
		return -1;
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
				acceptClient(serverFD, epollFD, log);
			}
			else if (events[i].data.ptr != NULL) {
				EpollDataWrapper* wrapper = events[i].data.ptr;
				if (wrapper->clientContextPtr->state != STATE_FORWARDING) {
					if (handleClientState(wrapper->clientContextPtr, epollFD, log) != 0) {
						deleteFromEpollSet(epollFD, wrapper->clientContextPtr->fd, &events[i]);
						freeEpollDataWrapper(wrapper);
					}
				} else if (wrapper->type == CLIENT) {
					if (forwardTrafficFromClient(wrapper->clientContextPtr, log) != 0) {
						deleteFromEpollSet(epollFD, wrapper->clientContextPtr->fd, &events[i]);
						freeEpollDataWrapper(wrapper);

					}
				} else if (wrapper->type == TARGET_SERVER) {
					if (forwardTrafficFromServer(wrapper->clientContextPtr, log) != 0) {
						deleteFromEpollSet(epollFD, wrapper->clientContextPtr->serverFD, &events[i]);
					}
				}
			} 
		}
	}

	close(serverFD);
	close(epollFD);
	return 0;
}

static int prepareServer(in_port_t port, Logger *log) {
	int serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD < 0) {
		logMessage(log, LOG_ERROR, "Failed to create server socket: %s", strerror(errno)); 
		return -1;
	}

	int opt = 1;
	if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	    logMessage(log, LOG_ERROR, "setsockopt(SO_REUSEADDR) failed");
	    close(serverFD);
	    return -1;
	}

	if(setSocketNonblocking(serverFD, log) == -1) {
		logMessage(log, LOG_ERROR, "Can't set serverFD nonblocking: %s", strerror(errno));
		return -1;
	}

	struct sockaddr_in serverAddr = {0};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = port;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		logMessage(log, LOG_ERROR, "Can't bind server socket: %s", strerror(errno));
		close(serverFD);
		return -1;
	}

	if (listen(serverFD, 10) < 0) {
		logMessage(log, LOG_ERROR, "Failed to listen socket: %s", strerror(errno));
		close(serverFD);
		return -1;
	} 

	return serverFD;
}

int startServer(in_port_t port, Logger *log) {
	int serverFD = prepareServer(port, log);
	if (serverFD <= 0) {
		return serverFD;
	} 

	if (startPolling(serverFD, log) == -1) {
		return -1;
	}
	return 0;
}
