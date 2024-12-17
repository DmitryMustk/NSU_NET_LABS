#include "../include/logger.h"
#include "../include/server.h"

#include "../include/dns_resolver.h"

#include <netinet/in.h>
#include <stdlib.h>

#define MAX_PORT_VALUE 65535


// TODO: 1. fix connect blocking (add separate event)
// TODO: 2. fix hello, connect fully reading
// TODO: 3. add dns failure client response
// TODO: 4. think of dns request resending
// TODO: 5. free context if and only if both nodes tcp-shutdown the connection
// TODO: 6. replace memset to non danger
// TODO: 7. dynamic buffer for domain
// TODO: 8. fix spining in forward traffic
// TODO: 9. think of unifying forward traffic

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
		return -1;
	}

	closeLogger(log);
	return 0;
}
