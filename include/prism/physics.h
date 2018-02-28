#pragma once

#include "geometry.h"

typedef Vector3D Velocity;
typedef Vector3D Acceleration;
typedef Vector3D Gravity;

typedef struct {
  Position mPosition;
  Velocity mVelocity;
  Acceleration mAcceleration;
} PhysicsObject;

void handlePhysics(PhysicsObject* tObject);
void setMaxVelocity(Velocity tVelocity);
void setMaxVelocityDouble(double tVelocity);
void resetMaxVelocity();
void setDragCoefficient(Vector3D tDragCoefficient);
void resetDragCoefficient();
void setGravity(Gravity tGravity);
Gravity getGravity();
void resetPhysicsObject(PhysicsObject* tObject);
void resetPhysics();
void initPhysics();
void pausePhysics();
void resumePhysics();

int isEmptyVelocity(Velocity tVelocity);
Velocity normalizeVelocity(Velocity tVelocity);

#define makeAcceleration(x, y, z) makePosition(x, y, z)

