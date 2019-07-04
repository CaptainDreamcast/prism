#pragma once

#include "blitztimelineanimationreader.h"
#include "actorhandler.h"

ActorBlueprint getBlitzTimelineAnimationHandler();

void addBlitzTimelineComponent(int tEntityID, BlitzTimelineAnimations* tAnimations);
int playBlitzTimelineAnimation(int tEntityID, int tAnimation);
void stopBlitzTimelineAnimation(int tEntityID, int tAnimationID);
void stopAllBlitzTimelineAnimations(int tEntityID);
void setBlitzTimelineAnimationCB(int tEntityID, int tAnimationID, int tCBID, void(*tCB)(void*), void * tCaller);