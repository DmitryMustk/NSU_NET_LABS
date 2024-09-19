#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>

void handleError(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void clearScreen(void) {
	printf("\033[H\033[J");
}

