#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include "logger.h"
#include "client_context.h"

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_CLIENTS 256

typedef struct {
    int udpSocket;
    struct sockaddr_in* dnsServerAddr;
    ClientContext* clientsArr[MAX_CLIENTS];
} DnsResolver;

DnsResolver* createDnsResolver(Logger* log);
void freeDnsResolver(DnsResolver* dnsResolver);
int sendDnsRequest(DnsResolver* dnsResolver, ClientContext* clientContext, const char* hostname, Logger* log);
ClientContext* getDnsResponse(DnsResolver* dnsResolver, char* ip, Logger* log);
#endif //DNS_RESOLVER_H
