#pragma once

#include "blitztimelineanimationreader.h"
#include "actorhandler.h"

extern ActorBlueprint BlitzTimelineAnimationHandler;

void addBlitzTimelineComponent(int tEntityID, BlitzTimelineAnimations* tAnimations);
int playBlitzTimelineAnimation(int tEntityID, int tAnimation);
void stopBlitzTimelineAnimation(int tEntityID, int tAnimationID);
void setBlitzTimelineAnimationCB(int tEntityID, int tAnimationID, void * tCB, void * tCaller);