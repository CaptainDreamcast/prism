#pragma once

#include "physics.h"

void setupPhysicsHandler();
void shutdownPhysicsHandler();

void updatePhysicsHandler();
int addToPhysicsHandler(Position tPosition);
void removeFromPhysicsHandler(int tID);
PhysicsObject* getPhysicsFromHandler(int tID);
Position* getHandledPhysicsPositionReference(int tID);
Velocity* getHandledPhysicsVelocityReference(int tID);
Acceleration* getHandledPhysicsAccelerationReference(int tID);
void addAccelerationToHandledPhysics(int tID, Acceleration tAccel);
void stopHandledPhysics(int tID);

void setHandledPhysicsMaxVelocity(int tID, double tVelocity);
void setHandledPhysicsDragCoefficient(int tID, Vector3D tDragCoefficient);
void setHandledPhysicsGravity(int tID, Vector3D tGravity);
