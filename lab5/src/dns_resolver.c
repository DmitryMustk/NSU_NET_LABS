#include "../include/dns_resolver.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DNS_SERVER_IP "8.8.8.8"
#define DNS_SERVER_PORT 53  

#define BUF_SIZE 512

static int addUdpSocket(DnsResolver* dnsResolver, Logger* log) {
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        logMessage(log, LOG_ERROR, "Can't create upd socket for resolver instance");
        return -1;
    }
    dnsResolver->udpSocket = udpSocket;
    return 0;
}

static int addDnsServerAddr(DnsResolver* dnsResolver, Logger* log) {
    if (dnsResolver->dnsServerAddr != NULL) {
        free(dnsResolver->dnsServerAddr);
    }
    dnsResolver->dnsServerAddr = malloc(sizeof(struct sockaddr_in));
    if (dnsResolver->dnsServerAddr == NULL) {
        logMessage(log, LOG_ERROR, "Can't allocate memory for dns server address for resolver instance");
        return -1;
    }

    dnsResolver->dnsServerAddr->sin_family = AF_INET;
    dnsResolver->dnsServerAddr->sin_port = htons(DNS_SERVER_PORT);
    inet_pton(AF_INET, DNS_SERVER_IP, &(dnsResolver->dnsServerAddr->sin_addr));
    return 0;
}

DnsResolver* createDnsResolver(Logger* log) {
    DnsResolver* dnsResolver = malloc(sizeof(DnsResolver));
    if (dnsResolver == NULL) {
        logMessage(log, LOG_ERROR, "Can't allocate memory for dns resolver instance");
        return NULL;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        dnsResolver->clientsArr[i] = NULL;
    }
    dnsResolver->dnsServerAddr = NULL;
    dnsResolver->udpSocket = -1;

    if (addUdpSocket(dnsResolver, log) == -1) {
        free(dnsResolver);
        return NULL;
    }

    if (addDnsServerAddr(dnsResolver, log) == -1) {
        close(dnsResolver->udpSocket);
        free(dnsResolver);
        return NULL;
    }

    return dnsResolver;
}

void freeDnsResolver(DnsResolver* dnsResolver) {
    close(dnsResolver->udpSocket);
    free(dnsResolver->dnsServerAddr);
    free(dnsResolver);
}

static int buildDnsQuery(uint16_t id, uint8_t* buf, const char* hostname, Logger* log) {
    memset(buf, 0, BUF_SIZE);
    buf[0] = (id >> 8) & 0xFF; 
    buf[1] = id & 0xFF;   
    buf[2] = 0x01;
    buf[5] = 0x01;

    uint8_t* hostnameCursor = buf + 12;
    const char* dot = NULL;
    while ((dot = strchr(hostname, '.')) != NULL) {
        *hostnameCursor++ = dot - hostname; // segment len
        memcpy(hostnameCursor, hostname, dot - hostname);
        hostnameCursor += dot - hostname;
        hostname = dot + 1;
    }

    *hostnameCursor++ = strlen(hostname); // the last segment
    memcpy(hostnameCursor, hostname, strlen(hostname));
    hostnameCursor += strlen(hostname);
    *hostnameCursor++ = 0; 

    buf[hostnameCursor - buf + 0] = 0x00; // A-type
    buf[hostnameCursor - buf + 1] = 0x01;
    buf[hostnameCursor - buf + 2] = 0x00; // IN
    buf[hostnameCursor - buf + 3] = 0x01;
    logMessage(log, LOG_DEBUG, "Build query of length: %d", hostnameCursor - buf + 4);
    return hostnameCursor - buf + 4; // query len  
}

// Returns clientContext id
static uint16_t parseDnsResponse(const char* buffer, char* ip, Logger* log) {
    // logHexMessage(log, LOG_DEBUG, buffer, BUF_SIZE);
    uint16_t id = ntohs(*(unsigned short*)buffer);
    logMessage(log, LOG_DEBUG, "Get dns response for %d ip");
    int questionsCount = ntohs(*(unsigned short*)(buffer + 4));
    int answersCount   = ntohs(*(unsigned short*)(buffer + 6));

    if (answersCount == 0) {
        logMessage(log, LOG_ERROR, "No answers in DNS response");
        return -1;
    }

    // skip header
    const char* cursor = buffer + 12;

    // skip questions
    while (questionsCount-- > 0) {
        while (*cursor != 0) {
            cursor++;
        }
        cursor += 5; // skip NULL + TYPE (2 bytes) + CLASS (2 bytes)
    }

    if ((*cursor & 0xC0) == 0xC0) {
        cursor += 2;
    } else {
        while (*cursor != 0) {
            cursor++;
        }
        cursor++;
    }
    // skip TYPE (2 bytes), CLASS (2 bytes), TTL (4 bytes) and data len (2 bytes)
    cursor += 10;

    // is A-record (IPv4)
    unsigned short dataLength = ntohs(*(unsigned short*)(cursor - 2));
    if (dataLength == 4) { // A-type
        snprintf(ip, INET_ADDRSTRLEN, "%u.%u.%u.%u",
                 (unsigned char)cursor[0], (unsigned char)cursor[1],
                 (unsigned char)cursor[2], (unsigned char)cursor[3]);
        return id;
    }

    logMessage(log, LOG_ERROR, "Non-A record in response");
    return -1;
}

ClientContext* getDnsResponse(DnsResolver* dnsResolver, char* ip, Logger* log) {
    char buffer[BUF_SIZE];
    socklen_t len = sizeof(*(dnsResolver->dnsServerAddr));
    int recvLen = recvfrom(dnsResolver->udpSocket, buffer, BUF_SIZE, 0, (struct sockaddr*)dnsResolver->dnsServerAddr, &len);
    logMessage(log, LOG_DEBUG, "Get from dns server response of %d bytes", recvLen);
    
    if (recvLen <= 0) {
        logMessage(log, LOG_ERROR, "Failed to get dns response");
        return NULL;
    }
    uint16_t id = parseDnsResponse(buffer, ip, log);
    if (id >= 0) {
        logMessage(log, LOG_INFO, "Domain successfuly resolved: %s for id: %d", ip, id);        
        if (dnsResolver->clientsArr[id] == NULL) {
            logMessage(log, LOG_ERROR, "Don't have client session for id: %d", id);
        }
        return dnsResolver->clientsArr[id];
    }

    logMessage(log, LOG_ERROR, "Failed to get dns response");
    return NULL;
}

int sendDnsRequest(DnsResolver* dnsResolver, ClientContext* clientContext, const char* hostname, Logger* log) {
    dnsResolver->clientsArr[clientContext->id] = clientContext;
    char dnsAddrStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(dnsResolver->dnsServerAddr->sin_addr), dnsAddrStr, INET_ADDRSTRLEN);
    uint8_t buffer[BUF_SIZE];

    int queryLen = buildDnsQuery(clientContext->id, buffer, hostname, log);

    if (sendto(dnsResolver->udpSocket, buffer, queryLen, 0, (struct sockaddr*)dnsResolver->dnsServerAddr, sizeof(struct sockaddr_in)) < 0) {
        logMessage(log, LOG_ERROR, "Can't send dns query to dns server: %s", strerror(errno));
        return -1;
    }

    logMessage(log, LOG_INFO, "Successfully sended dns query for [%s] domain", hostname);
    return 0;
}
