#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "logger.h"

void acceptClient(int serverFD, int epollFD, Logger* log);
void handleClient(int clientFD, int epollFD, Logger* log);

#endif //CLIENT_HANDLER_H
