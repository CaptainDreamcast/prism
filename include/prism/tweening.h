#pragma once

#include "animation.h"
#include "actorhandler.h"

typedef double(*TweeningFunction)(double t);
typedef void(*TweeningCBFunction)(void* tCaller);

ActorBlueprint getTweeningHandler();

int tweenDouble(double* tDst, double tStart, double tEnd, TweeningFunction tFunc, Duration tDuration, TweeningCBFunction tCB, void* tCaller);
void removeTween(int tID);

double linearTweeningFunction(double t);
double quadraticTweeningFunction(double t);
double inverseQuadraticTweeningFunction(double t);
double squareRootTweeningFunction(double t);
double overshootTweeningFunction(double t);
double transformAtEndTweeningFunction(double t);