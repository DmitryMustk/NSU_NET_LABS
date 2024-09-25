#ifndef LAB1_IPV6_WRAPS
#define LAB1_IPV6_WRAPS

#include <bits/types/struct_iovec.h>
#include <netinet/in.h>

void configureAddress6(struct sockaddr_in6* addr, const char* address, in_port_t port);
void bindSocket6(int sockfd, const char* address, in_port_t port);
void joinMCGroup6(int sockfd, const char* address);
int getAddressStr6(struct sockaddr* address, char* strBuf, size_t bufSize);
void autoBindSocket6(int sockfd, const char* peekAddr, in_port_t port);

#endif //LAB1_IPV6_WRAPS
