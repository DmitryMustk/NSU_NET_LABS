#ifndef SERVER_H
#define SERVER_H
#include "logger.h"
#include "client_context.h"

#include <netinet/in.h>

int startServer(in_port_t port, Logger* logger);

#endif //SERVER_H
