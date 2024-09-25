#include "../include/utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleError(const char *msg) {
	printf("%d", errno);
	perror(msg);
	exit(EXIT_FAILURE);
}

void clearScreen(void) {
	printf("\033[H\033[J");
}

