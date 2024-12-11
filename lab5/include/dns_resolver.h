#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include "logger.h"

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct {
    int udpSocket;
    struct sockaddr_in* dnsServerAddr;
} DnsResolver;

DnsResolver* createDnsResolver(Logger* log);
void freeDnsResolver(DnsResolver* dnsResolver);
int sendDnsRequest(DnsResolver* dnsResolver, const char* hostname, Logger* log);
int getDnsResponse(DnsResolver* dnsResolver, char* ip, Logger* log);
#endif //DNS_RESOLVER_H
