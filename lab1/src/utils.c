#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>

void handleError(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}
