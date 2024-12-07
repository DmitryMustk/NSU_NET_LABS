#ifndef SOCKS5_H
#define SOCKS5_H

#include "logger.h"
#include "client_context.h"

int processSocks5(ClientContext* clientContext, Logger* log);

#endif //SOCKS5_H
