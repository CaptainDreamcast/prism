#pragma once

#include "blitztimelineanimationreader.h"
#include "actorhandler.h"

extern ActorBlueprint BlitzTimelineAnimationHandler;

void addBlitzTimelineComponent(int tEntityID, BlitzTimelineAnimations* tAnimations);
void playBlitzTimelineAnimation(int tEntityID, int tAnimation);