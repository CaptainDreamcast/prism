#ifndef TARI_PHYSICSHANDLER_H
#define TARI_PHYSICSHANDLER_H

#include "physics.h"

fup void setupPhysicsHandler();
fup void shutdownPhysicsHandler();

fup void updatePhysicsHandler();
fup int addToPhysicsHandler(Position tPosition);
fup void removeFromPhysicsHandler(int tID);
fup PhysicsObject* getPhysicsFromHandler(int tID);
fup void addAccelerationToHandledPhysics(int tID, Acceleration tAccel);

fup void setHandledPhysicsMaxVelocity(int tID, double tVelocity);
fup void setHandledPhysicsDragCoefficient(int tID, Vector3D tDragCoefficient);
fup void setHandledPhysicsGravity(int tID, Vector3D tGravity);
#endif
