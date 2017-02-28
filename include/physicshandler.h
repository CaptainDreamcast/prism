#ifndef TARI_PHYSICSHANDLER_H
#define TARI_PHYSICSHANDLER_H

#include "physics.h"

void setupPhysicsHandler();
void shutdownPhysicsHandler();

void updatePhysicsHandler();
int addToPhysicsHandler(Position tPosition);
void removeFromPhysicsHandler(int tID);
PhysicsObject* getPhysicsFromHandler(int tID);
void addAccelerationToHandledPhysics(int tID, Acceleration tAccel);

void setHandledPhysicsMaxVelocity(int tID, double tVelocity);
void setHandledPhysicsDragCoefficient(int tID, Vector3D tDragCoefficient);
void setHandledPhysicsGravity(int tID, Vector3D tGravity);
#endif
