#ifndef TARI_TIMER
#define TARI_TIMER

#include "animation.h"

typedef void (*TimerCB)(void* caller);

int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller);

void setupTimer();
void updateTimer();
void clearTimer();
void shutdownTimer();

#endif
