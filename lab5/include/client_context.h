#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <stdint.h>
#include <netinet/in.h>

typedef enum {
    STATE_NONE,
    STATE_HELLO,
    STATE_CONNECTION,
    STATE_FORWARDING,
} ClientState;

typedef enum {
    IPV4,
    IPV6,
    DOMAIN,
} AddrType;

typedef struct {
    int         id;
    int         fd;
    int         serverFD;
    char        serverIP[INET_ADDRSTRLEN];
    in_port_t   serverPort;
    int         isDomainResolved;
    AddrType    addrType;
    ClientState state;
} ClientContext;

typedef enum {
    CLIENT,
    TARGET_SERVER,
} SocketType;

typedef struct {
    SocketType     type;
    ClientContext* clientContextPtr;
} EpollDataWrapper;

ClientContext* createClientContext(int fd);
EpollDataWrapper* createEpollDataWrapper(ClientContext* clientContext, SocketType type);
void freeEpollDataWrapper(EpollDataWrapper* epollDataWrapper);
void freeClientContext(ClientContext * clientContext);

#endif//CLIENT_CONTEXT_H
