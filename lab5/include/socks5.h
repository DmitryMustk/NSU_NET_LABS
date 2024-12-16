#ifndef SOCKS5_H
#define SOCKS5_H

#include "logger.h"
#include "client_context.h"
#include "dns_resolver.h"

// int sendSocks5ConnectResponse(ClientContext* clientContext, Logger* log);
int handleClientState(ClientContext* clientContext, int epollFD, DnsResolver* dnsResolver, Logger* log);

#endif //SOCKS5_H
