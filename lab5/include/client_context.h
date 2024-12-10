#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <stdint.h>

typedef enum {
    STATE_NONE,
    STATE_HELLO,
    STATE_CONNECTION,
    STATE_FORWARDING,
} ClientState;

typedef struct {
    int fd;
    int serverFD;
    ClientState state;
} ClientContext;

typedef enum {
    CLIENT,
    TARGET_SERVER,
} SocketType;

typedef struct {
    SocketType type;
    ClientContext* clientContextPtr;
} EpollDataWrapper;

ClientContext* createClientContext(int fd);
EpollDataWrapper* createEpollDataWrapper(ClientContext* clientContext, SocketType type);
void freeEpollDataWrapper(EpollDataWrapper* epollDataWrapper);
void freeClientContext(ClientContext * clientContext);

#endif//CLIENT_CONTEXT_H
