#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include "logger.h"

#include <stdint.h>

int resolveDomainNonBlocking(const char* domain, int clientFD, uint16_t clientPort, Logger* log);

#endif //DNS_RESOLVER_H
