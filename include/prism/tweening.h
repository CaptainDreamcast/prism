#pragma once

#include "animation.h"
#include "actorhandler.h"

namespace prism {

typedef float(*TweeningFunction)(float t);
typedef void(*TweeningCBFunction)(void* tCaller);

ActorBlueprint getTweeningHandler();

int tweenDouble(float* tDst, float tStart, float tEnd, TweeningFunction tFunc, Duration tDuration, TweeningCBFunction tCB, void* tCaller);
void removeTween(int tID);

float linearTweeningFunction(float t);
float quadraticTweeningFunction(float t);
float inverseQuadraticTweeningFunction(float t);
float squareRootTweeningFunction(float t);
float overshootTweeningFunction(float t);
float transformAtEndTweeningFunction(float t);

void imguiTweeningHandler();

}