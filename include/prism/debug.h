#pragma once

#include <prism/actorhandler.h>

int isInDevelopMode();

ActorBlueprint getPrismDebug();

void setPrismDebugUpdateStartTime();
void setPrismDebugDrawingStartTime();
void setPrismDebugWaitingStartTime();
