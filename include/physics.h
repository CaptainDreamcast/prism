#ifndef TARI_PHYSICS
#define TARI_PHYSICS

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
void setGravity(Gravity tGravity);
void resetPhysicsObject(PhysicsObject* tObject);
void resetPhysics();
void initPhysics();
void pausePhysics();
void resumePhysics();

int isEmptyVelocity(Velocity tVelocity);
Velocity normalizeVelocity(Velocity tVelocity);

#endif
