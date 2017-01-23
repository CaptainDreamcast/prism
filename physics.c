#include "include/physics.h"

#include <string.h>
#include <math.h>
#include <kos.h>

#include "include/log.h"
#include "include/framerate.h"

static Velocity gMaxVelocity;

void setMaxVelocity(Velocity tVelocity) {
  gMaxVelocity = tVelocity;
}

static Gravity gGravity;

void setGravity(Gravity tGravity) {
  gGravity = tGravity;
}

double fmin(double a, double b) {
  return (a < b) ? a : b;
}

double fmax(double a, double b) {
  return (a > b) ? a : b;
}

void handlePhysics(PhysicsObject* tObject) {
  tObject->mPosition.x += tObject->mVelocity.x;
  tObject->mPosition.y += tObject->mVelocity.y;
  tObject->mPosition.z += tObject->mVelocity.z;

  double f = getFramerateFactor();
  tObject->mVelocity.x += tObject->mAcceleration.x*f;
  tObject->mVelocity.y += tObject->mAcceleration.y*f;
  tObject->mVelocity.z += tObject->mAcceleration.z*f;

  //TODO: fix velocity increase problem for 50Hz
  tObject->mVelocity.x = fmax(-gMaxVelocity.x*f, fmin(gMaxVelocity.x*f, tObject->mVelocity.x));
  tObject->mVelocity.y = fmax(-gMaxVelocity.y, fmin(gMaxVelocity.y, tObject->mVelocity.y));
  tObject->mVelocity.z = fmax(-gMaxVelocity.x, fmin(gMaxVelocity.z, tObject->mVelocity.z));

  tObject->mAcceleration.x = gGravity.x;
  tObject->mAcceleration.y = gGravity.y;
  tObject->mAcceleration.z = gGravity.z;
}

void resetPhysicsObject(PhysicsObject* tObject) {
  memset(tObject, 0, sizeof(*tObject));
}

void resetPhysics() {
  Velocity maxVelocity;
  maxVelocity.x = INFINITY;
  maxVelocity.y = INFINITY;
  maxVelocity.z = INFINITY;
  setMaxVelocity(maxVelocity);

  Gravity gravity;
  gravity.x = 0;
  gravity.y = 0;
  gravity.z = 0;
  setGravity(gravity);
}

void initPhysics() {
  resetPhysics();
}

int isEmptyVelocity(Velocity tVelocity) {
  return tVelocity.x == 0 && tVelocity.y == 0 && tVelocity.z == 0;
}
Velocity normalizeVelocity(Velocity tVelocity) {
  double l = vecLength(tVelocity);
  if (l == 0) {
    logError("Empty vector length; return original vector");
    return tVelocity;
  }

  tVelocity.x /= l;
  tVelocity.y /= l;
  tVelocity.z /= l;

  return tVelocity;
}


