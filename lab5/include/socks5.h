#ifndef SOCKS5_H
#define SOCKS5_H

#include "logger.h"
#include "client_context.h"

int handleClientState(ClientContext* clientContext, int epollFD, Logger* log);

#endif //SOCKS5_H
