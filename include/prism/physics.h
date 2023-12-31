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
void setMaxVelocity(const Velocity& tVelocity);
void setMaxVelocityDouble(double tVelocity);
void resetMaxVelocity();
void setDragCoefficient(const Vector3D& tDragCoefficient);
void resetDragCoefficient();
void setGravity(const Gravity& tGravity);
Gravity getGravity();
void resetPhysicsObject(PhysicsObject* tObject);
void resetPhysics();
void initPhysics();
void pausePhysics();
void resumePhysics();

int isEmptyVelocity(const Velocity& tVelocity);
Velocity normalizeVelocity(const Velocity& tVelocity);

#ifdef _WIN32
void imguiPhysics();
#endif

#define makeAcceleration(x, y, z) Vector3D(x, y, z)
