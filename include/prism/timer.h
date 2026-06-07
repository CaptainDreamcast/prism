#pragma once

#include "animation.h"

namespace prism {

typedef void (*TimerCB)(void* caller);

int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller);
void removeTimer(int tID);

void setupTimer();
void updateTimer();
void clearTimer();
void shutdownTimer();

int hasTimerFinished(int tID);

void imguiTimer();

}