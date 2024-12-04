#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <pthread.h>

typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;

typedef struct {
    FILE* file;
    LogLevel level;
    int toConsole;
    pthread_mutex_t mutex;
} Logger;

Logger* createLogger(const char* filename, LogLevel level, int toConsole);
void logMessage(Logger* logger, LogLevel level, const char* format, ...);
void closeLogger(Logger* logger);

#endif // LOGGER_H
