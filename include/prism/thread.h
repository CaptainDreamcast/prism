#pragma once

#include "actorhandler.h"

typedef void* Semaphore;

void initThreading();
void shutdownThreading();
int startThread(void(tFunc)(void*), void* tCaller);
Semaphore createSemaphore(int tInitialAccessesAllowed);
void destroySemaphore(Semaphore tSemaphore);
void lockSemaphore(Semaphore tSemaphore);
void releaseSemaphore(Semaphore tSemaphore); 
void terminateSelfAsThread(int tReturnValue);