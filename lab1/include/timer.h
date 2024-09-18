#ifndef LAB1_TIMER_H
#define LAB1_TIMER_H
#include <time.h>

typedef struct {
	int seconds;
	time_t start;
} Timer;

void startTimer(Timer* timer, int sec);
int timerExpired(const Timer* timer);
void resetTimer(Timer* timer);

#endif //LAB1_TIMER_H
