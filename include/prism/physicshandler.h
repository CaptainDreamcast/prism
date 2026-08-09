#pragma once

#include "physics.h"

namespace prism {

struct PhysicsHandlerElement {
	int mID;
	PhysicsObject mObj;
	float mMaxVelocity;
	Vector3D mDragCoefficient;
	Gravity mGravity;
	int mIsPaused;

	float mTimeDilatationNow;
	float mTimeDilatation;
};

void setupPhysicsHandler();
void shutdownPhysicsHandler();

void updatePhysicsHandler();
PhysicsHandlerElement* addToPhysicsHandler(const Position& tPosition);
void removeFromPhysicsHandler(PhysicsHandlerElement* tElement);
PhysicsObject* getPhysicsFromHandler(PhysicsHandlerElement* tElement);
Position getHandledPhysicsPosition(PhysicsHandlerElement* tElement);
Position* getHandledPhysicsPositionReference(PhysicsHandlerElement* tElement);
Velocity* getHandledPhysicsVelocityReference(PhysicsHandlerElement* tElement);
Acceleration* getHandledPhysicsAccelerationReference(PhysicsHandlerElement* tElement);
void addAccelerationToHandledPhysics(PhysicsHandlerElement* tElement, const Acceleration& tAccel);
void stopHandledPhysics(PhysicsHandlerElement* tElement);
void pauseHandledPhysics(PhysicsHandlerElement* tElement);
void resumeHandledPhysics(PhysicsHandlerElement* tElement);

void setHandledPhysicsMaxVelocity(PhysicsHandlerElement* tElement, float tVelocity);
void setHandledPhysicsDragCoefficient(PhysicsHandlerElement* tElement, const Vector3D& tDragCoefficient);
void setHandledPhysicsGravity(PhysicsHandlerElement* tElement, const Vector3D& tGravity);
void setHandledPhysicsSpeed(PhysicsHandlerElement* tElement, float tSpeed);

void imguiPhysicsHandler();

}