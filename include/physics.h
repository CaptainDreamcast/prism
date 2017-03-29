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

fup void handlePhysics(PhysicsObject* tObject);
fup void setMaxVelocity(Velocity tVelocity);
fup void setMaxVelocityDouble(double tVelocity);
fup void resetMaxVelocity();
fup void setDragCoefficient(Vector3D tDragCoefficient);
fup void resetDragCoefficient();
fup void setGravity(Gravity tGravity);
fup Gravity getGravity();
fup void resetPhysicsObject(PhysicsObject* tObject);
fup void resetPhysics();
fup void initPhysics();
fup void pausePhysics();
fup void resumePhysics();

fup int isEmptyVelocity(Velocity tVelocity);
fup Velocity normalizeVelocity(Velocity tVelocity);

#define makeAcceleration(x, y, z) makePosition(x, y, z)

#endif
