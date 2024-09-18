#include "../include/timer.h"

#include <time.h>

void startTimer(Timer *timer, int sec) {
	timer->seconds = sec;
	timer->start = time(NULL);
}

int timerExpired(const Timer *timer) {
	return difftime(time(NULL), timer->start) >= timer->seconds;
}

void resetTimer(Timer *timer) {
	timer->start = time(NULL);
}
