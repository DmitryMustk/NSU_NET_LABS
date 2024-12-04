#include "../include/logger.h"

#include <stdarg.h>
#include <stdlib.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_RED     "\033[0;31m"

static const char* levelToString(LogLevel level) {
    switch (level) {
        case LOG_INFO: return "INFO";
        case LOG_ERROR: return "ERROR";
        case LOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

Logger* createLogger(const char* filename, LogLevel level, int toConsole) {
    Logger* logger = (Logger*)malloc(sizeof(Logger));
    if (!logger) {
        perror("[ERROR] Failed to allocate memory for logger");
        return NULL;
    }
    logger->file = fopen(filename, "a");
    if (!logger->file) {
        perror("[ERROR] Failed to open log file");
        free(logger);
        return NULL;
    }
    logger->level = level;
    logger->toConsole = toConsole;
    pthread_mutex_init(&logger->mutex, NULL);
    return logger;
}

void logMessage(Logger* logger, LogLevel level, const char* format, ...) {
    if (level > logger->level) {
        return;
    }

    pthread_mutex_lock(&logger->mutex);

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    char header[64];
    snprintf(header, sizeof(header), "[%02d-%02d-%02d %02d:%02d:%02d] %s: ",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec, levelToString(level));

    va_list args;
    va_start(args, format);
    fprintf(logger->file, "%s", header);
    vfprintf(logger->file, format, args);
    fprintf(logger->file, "\n");
    fflush(logger->file);
    va_end(args);

    if (logger->toConsole) {
        const char* color = COLOR_RESET;
        if (level == LOG_INFO) {
            color = COLOR_GREEN;
        } else if (level == LOG_ERROR) {
            color = COLOR_RED;
        }

        va_start(args, format);
        fprintf((level == LOG_ERROR) ? stderr : stdout, "%s%s", color, header);
        vfprintf((level == LOG_ERROR) ? stderr : stdout, format, args);
        fprintf((level == LOG_ERROR) ? stderr : stdout, "%s\n", COLOR_RESET);
        va_end(args);
    }

    pthread_mutex_unlock(&logger->mutex);
}

void closeLogger(Logger* logger) {
    if (logger) {
        fclose(logger->file);
        pthread_mutex_destroy(&logger->mutex);
        free(logger);
    }
}
