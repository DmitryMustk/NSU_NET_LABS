#ifndef IPV4_WRAPS_H
#define IPV4_WRAPS_H

#include <arpa/inet.h>

void configureAddress(struct sockaddr_in* addr, const char* address, in_port_t port);
void bindSocket(int sockfd, const char* address, in_port_t port);
void joinMCGroup(int sockfd, const char* address);
int getAddressStr(struct sockaddr* address, char* strBuf, size_t bufSize);
void autoBindSocket(int sockfd, const char* peekAddr, in_port_t peekPort);

#endif //IPV4_WRAPS_H 

