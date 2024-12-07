#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "logger.h"
#include "client_context.h"

void acceptClient(int serverFD, int epollFD, Logger* log);
void handleClient(ClientContext* clientContext, int epollFD, Logger* log);

#endif //CLIENT_HANDLER_H
