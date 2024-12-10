#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

typedef enum {
    STATE_NONE,
    STATE_HELLO,
    STATE_CONNECTION,
    STATE_FORWARDING,
} ClientState;

typedef struct {
    int fd;
    int serverFD;
    int isServerFDPolling;
    ClientState state;
} ClientContext;

ClientContext* createClientContext(int fd);
void freeClientContext(ClientContext * clientContext);

#endif//CLIENT_CONTEXT_H
