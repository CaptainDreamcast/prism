#pragma once

#include "tari/animation.h"

typedef double(*TweeningFunction)(double t);
typedef void(*TweeningCBFunction)(void* tCaller);

fup void setupTweening();
fup void shutdownTweening();
fup void updateTweening();

fup int tweenDouble(double* tDst, double tStart, double tEnd, TweeningFunction tFunc, Duration tDuration, TweeningCBFunction tCB, void* tCaller);
fup void removeTween(int tID);

fup double linearTweeningFunction(double t);
fup double quadraticTweeningFunction(double t);
fup double overshootTweeningFunction(double t);
fup double transformAtEndTweeningFunction(double t);