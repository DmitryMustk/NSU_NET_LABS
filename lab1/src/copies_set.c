#include "../include/copies_set.h"
#include "../include/utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static void removeCopyFromCopiesSet(CopiesSet* copiesSet, int index) {
	for (int i = index; i < copiesSet->capacity - 1; ++i) {
		copiesSet->copiesArr[i] = copiesSet->copiesArr[i + 1];
	}

	copiesSet->size--;
}

void appendToCopiesSet(CopiesSet* copiesSet, Copy* copy) {
	if (copiesSet->size >= copiesSet->capacity - 1) {
		return;
	}

	for (int i = 0; i < copiesSet->size; ++i) {
		if (strcmp(copiesSet->copiesArr[i].name, copy->name) == 0) {
			copiesSet->copiesArr[i].lastSeen = time(NULL);
			return;
		} 
	}
	copiesSet->copiesArr[copiesSet->size] = *copy;
	copiesSet->size++;
}

void removeDeadCopies(CopiesSet* copiesSet) {
	for (int i = 0; i < copiesSet->size; ++i) {
		if (difftime(time(NULL), copiesSet->copiesArr[i].lastSeen) >= COPY_TTL_SECS) {
			removeCopyFromCopiesSet(copiesSet, i);	
			i--;
		}
	}
}

void printCopiesSet(CopiesSet* copiesSet) {
	clearScreen();
	printf("Active copies:\n");

	for (int i = 0; i < copiesSet->size; ++i) {
		printf("%d. %s\n", i + 1, copiesSet->copiesArr[i].name);
	}
}
