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
    struct sockaddr_in* dnsServerAddr = malloc(sizeof(struct sockaddr_in));
    if (dnsServerAddr == NULL) {
        logMessage(log, LOG_ERROR, "Can't allocate memory for dns server address for resolver instance");
        return -1;
    }

    dnsServerAddr->sin_family = AF_INET;
    dnsServerAddr->sin_port = htons(DNS_SERVER_PORT);
    inet_pton(AF_INET, DNS_SERVER_IP, &dnsServerAddr->sin_addr);
    dnsResolver->dnsServerAddr = dnsServerAddr;
    return 0;
}

DnsResolver* createDnsResolver(Logger* log) {
    DnsResolver* dnsResolver = malloc(sizeof(DnsResolver));
    if (dnsResolver == NULL) {
        logMessage(log, LOG_ERROR, "Can't allocate memory for dns resolver instance");
        return NULL;
    }

    if (addUdpSocket(dnsResolver, log) == -1) {
        free(dnsResolver);
        return NULL;
    }

    if (addDnsServerAddr(dnsResolver, log) == -1) {
        close(dnsResolver->udpSocket);
        free(dnsResolver);
    }

    return dnsResolver;
}

void freeDnsResolver(DnsResolver* dnsResolver) {
    close(dnsResolver->udpSocket);
    free(dnsResolver->dnsServerAddr);
    free(dnsResolver);
}

static int buildDnsQuery(uint8_t* buf, const char* hostname, Logger* log) {
    memset(buf, 0, BUF_SIZE);

    buf[0] = 0x12;
    buf[1] = 0x34;
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

static int parseDnsResponse(const char* buffer, char* ip, Logger* log) {
    // logHexMessage(log, LOG_DEBUG, buffer, BUF_SIZE);
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
        return 0;
    }

    logMessage(log, LOG_ERROR, "Non-A record in response");
    return -1;
}

int getDnsResponse(DnsResolver* dnsResolver, char* ip, Logger* log) {
    char buffer[BUF_SIZE];
    socklen_t len = sizeof(dnsResolver->dnsServerAddr);
    int recvLen = recvfrom(dnsResolver->udpSocket, buffer, BUF_SIZE, 0, (struct sockaddr*)&dnsResolver->dnsServerAddr, &len);
    logMessage(log, LOG_DEBUG, "Get from dns server response of %d bytes", recvLen);

    if (recvLen > 0 && parseDnsResponse(buffer, ip, log) == 0) {
        logMessage(log, LOG_INFO, "Domain successfuly resolved: %s", ip);
        return 0;
    }

    return -1;
}

int sendDnsRequest(DnsResolver* dnsResolver, const char* hostname, Logger* log) {
    uint8_t buffer[BUF_SIZE];

    int queryLen = buildDnsQuery(buffer, hostname, log);
    if (sendto(dnsResolver->udpSocket, buffer, queryLen, 0, (struct sockaddr*)dnsResolver->dnsServerAddr, sizeof(*(dnsResolver->dnsServerAddr))) < 0) {
        logMessage(log, LOG_ERROR, "Can't send dns query to dns server: %s", strerror(errno));
        return -1;
    }

    logMessage(log, LOG_INFO, "Successfully sended dns query for [%s] domain", hostname);
    return 0;
}
