#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "logger.h"
#include "client_context.h"


int connectToTargetServer(ClientContext* clientContext, Logger* log);
int acceptClient(int serverFD, int epollFD, Logger* log);
int forwardTrafficFromClient(ClientContext* clientContext, Logger* log);
int forwardTrafficFromServer(ClientContext* clientContext, Logger* log);

#endif //CLIENT_HANDLER_H
