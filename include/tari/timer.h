#ifndef TARI_TIMER
#define TARI_TIMER

#include "animation.h"

typedef void (*TimerCB)(void* caller);

fup int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller);
fup void removeTimer(int tID);

fup void setupTimer();
fup void updateTimer();
fup void clearTimer();
fup void shutdownTimer();

#endif
