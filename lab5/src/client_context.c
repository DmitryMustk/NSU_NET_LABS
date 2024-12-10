#include "../include/client_context.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

ClientContext* createClientContext(int fd) {
    ClientContext* clientContext = malloc(sizeof(ClientContext));
    if (clientContext == NULL) {
        return NULL;
    }

    clientContext->fd = fd;
    clientContext->serverFD = 0;
    clientContext->state = STATE_NONE;
    return clientContext;
}

EpollDataWrapper* createEpollDataWrapper(ClientContext* clientContext, SocketType type) {
    EpollDataWrapper* epollDataWrapper = malloc(sizeof(EpollDataWrapper));
    if (epollDataWrapper == NULL) {
        return NULL;
    }
    epollDataWrapper->clientContextPtr = clientContext;
    epollDataWrapper->type = type;
    return epollDataWrapper;
}

void freeEpollDataWrapper(EpollDataWrapper* epollDataWrapper) {
    if (epollDataWrapper == NULL) {
        return;
    }
    freeClientContext(epollDataWrapper->clientContextPtr);
    free(epollDataWrapper);
}

void freeClientContext(ClientContext * clientContext) {
    if (clientContext == NULL) {
        return;
    }
    close(clientContext->fd);
    close(clientContext->serverFD);
    free(clientContext);
}
