#include "../include/epoll_wrapper.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

int createEpollInstance(Logger *log) {
    int epollFD = epoll_create1(0);
    if (epollFD < 0) {
		logMessage(log, LOG_ERROR, "Failed to create epoll: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
    return epollFD;
}

int addToEpollSetByFD(int epollFD, int fd, int events) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    return epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
}

int addToEpollSetByPtr(int epollFD, int fd, void* ptr, int events) {
    struct epoll_event event;
    event.data.ptr = ptr;
    event.events = events;

    return epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
}

int deleteFromEpollSet(int epollFD, int fd, struct epoll_event* event) {
    return epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, event);
}
