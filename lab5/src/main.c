#include "../include/logger.h"
#include "../include/server.h"

#include <netinet/in.h>
#include <stdlib.h>

#define MAX_PORT_VALUE 65535

// TODO: DNS

int main(int argc, char** argv) {
	Logger* log = createLogger("log.txt", LOG_DEBUG, 1);
	if (argc != 2) {
		logMessage(log, LOG_ERROR, "Usage: %s <port>", argv[0]);
		return -1;
	}
	
	in_port_t port = htons(atoi(argv[1]));
	if (ntohs(port) != atoi(argv[1]) || atoi(argv[1]) == 0) {
		logMessage(log, LOG_ERROR, "Port value must be between (0, %d)", MAX_PORT_VALUE);
		return -1;
	}

	logMessage(log, LOG_INFO, "socksProxy started on %s port...", argv[1]); 
	logMessage(log, LOG_INFO, "No Auth method required");

	if (startServer(port, log) < 0) {
		logMessage(log, LOG_ERROR, "Failed to start server");
		return 1-;
	}

	closeLogger(log);
	return 0;
}
