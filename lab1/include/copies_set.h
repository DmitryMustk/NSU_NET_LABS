#ifndef LAB1_COPIES_SET_H
#define LAB1_COPIES_SET_H

#include <netinet/in.h>
#include <time.h>

#define PORT_LEN 5
#define MAX_COPIES 128
#define COPY_TTL_SECS 3 

typedef struct {
	char name[INET_ADDRSTRLEN + PORT_LEN + 2];
	time_t lastSeen;
} Copy;

typedef struct {
	int size;
	int capacity;

	Copy copiesArr[MAX_COPIES];
} CopiesSet;

void appendToCopiesSet(CopiesSet* copiesSet, Copy* copy);
void removeDeadCopies(CopiesSet* copiesSet);
void printCopiesSet(CopiesSet* copiesSet);

#endif //LAB1_COPIES_SET_H
