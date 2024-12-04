#include "../include/logger.h"

#define ERROR -1

int main(int argc, char** argv) {
	Logger* log = createLogger("log.txt", LOG_DEBUG, 1);
	if (argc != 2) {
		logMessage(log, LOG_ERROR, "Usage: %s <port>", argv[0]);
		return ERROR;
	}

	logMessage(log, LOG_INFO, "socksProxy started on %s port...", argv[1]); 
	closeLogger(log);
	return 0;
}
