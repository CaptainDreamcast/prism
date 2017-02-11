#ifndef TARI_PHYSICSHANDLER_H
#define TARI_PHYSICSHANDLER_H

#include "physics.h"

void setupPhysicsHandler();
void shutdownPhysicsHandler();

void updatePhysicsHandler();
int addToPhysicsHandler(Position tPosition);
PhysicsObject* getPhysicsFromHandler(int tID);

#endif
