#pragma once

#include "physics.h"

struct PhysicsHandlerElement {
	int mID;
	PhysicsObject mObj;
	double mMaxVelocity;
	Vector3D mDragCoefficient;
	Gravity mGravity;
	int mIsPaused;

	double mTimeDilatationNow;
	double mTimeDilatation;
};

void setupPhysicsHandler();
void shutdownPhysicsHandler();

void updatePhysicsHandler();
PhysicsHandlerElement* addToPhysicsHandler(Position tPosition);
void removeFromPhysicsHandler(PhysicsHandlerElement* tElement);
PhysicsObject* getPhysicsFromHandler(PhysicsHandlerElement* tElement);
Position* getHandledPhysicsPositionReference(PhysicsHandlerElement* tElement);
Velocity* getHandledPhysicsVelocityReference(PhysicsHandlerElement* tElement);
Acceleration* getHandledPhysicsAccelerationReference(PhysicsHandlerElement* tElement);
void addAccelerationToHandledPhysics(PhysicsHandlerElement* tElement, Acceleration tAccel);
void stopHandledPhysics(PhysicsHandlerElement* tElement);
void pauseHandledPhysics(PhysicsHandlerElement* tElement);
void resumeHandledPhysics(PhysicsHandlerElement* tElement);

void setHandledPhysicsMaxVelocity(PhysicsHandlerElement* tElement, double tVelocity);
void setHandledPhysicsDragCoefficient(PhysicsHandlerElement* tElement, Vector3D tDragCoefficient);
void setHandledPhysicsGravity(PhysicsHandlerElement* tElement, Vector3D tGravity);
void setHandledPhysicsSpeed(PhysicsHandlerElement* tElement, double tSpeed);
