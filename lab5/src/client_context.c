#include "../include/client_context.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

uint32_t globalId = 0;

ClientContext* createClientContext(int fd) {
    ClientContext* clientContext = malloc(sizeof(ClientContext));
    if (!clientContext) {
        return NULL;
    }

    clientContext->id = globalId++;
    clientContext->fd = fd;
    clientContext->serverFD = 0;
    clientContext->isServerFDPolling = 0;
    clientContext->state = STATE_NONE;
    return clientContext;
}

void freeClientContext(ClientContext * clientContext) {
    if (!clientContext) {
        return;
    }
    close(clientContext->fd);
    close(clientContext->serverFD);
    free(clientContext);
}