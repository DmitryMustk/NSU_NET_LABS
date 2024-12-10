#ifndef EPOLL_WRAPPER_H
#define EPOLL_WRAPPER_H

#include "logger.h"

#include <sys/epoll.h>

int createEpollInstance(Logger* log);
int addToEpollSetByFD(int epollFD, int fd, int events);
int addToEpollSetByPtr(int epollFD, int fd, void* ptr, int events);
int deleteFromEpollSet(int epollFD, int fd, struct epoll_event* event);
#endif//EPOLL_WRAPPER_H
