#include "include/physics.h"

#include <string.h>
#include <math.h>

#include "include/log.h"
#include "include/framerate.h"

static struct {
	Velocity mMaxVelocity;
	Gravity mGravity;
	int mIsPaused;
	Vector3D mOneMinusDragCoefficient;
} gData;

void setMaxVelocity(Velocity tVelocity) {
  gData.mMaxVelocity = tVelocity;
}

void setMaxVelocityDouble(double tVelocity) {
   gData.mMaxVelocity = makePosition(tVelocity, tVelocity, tVelocity); // TODO: change this here and below to enforce vector length tVelocity
}

void resetMaxVelocity() {
  Velocity maxVelocity;
  maxVelocity.x = INFINITY;
  maxVelocity.y = INFINITY;
  maxVelocity.z = INFINITY;
  setMaxVelocity(maxVelocity);
}

void setDragCoefficient(Vector3D tDragCoefficient) {
  gData.mOneMinusDragCoefficient = vecAdd(makePosition(1, 1, 1), vecScale(tDragCoefficient, -1));
}

void resetDragCoefficient() {
	gData.mOneMinusDragCoefficient = makePosition(1,1,1);
}

void setGravity(Gravity tGravity) {
  gData.mGravity = tGravity;
}

Gravity getGravity() {
	return gData.mGravity;
}

double fmin(double a, double b) {
  return (a < b) ? a : b;
}

double fmax(double a, double b) {
  return (a > b) ? a : b;
}

void handlePhysics(PhysicsObject* tObject) {
  if(gData.mIsPaused) return;
  tObject->mPosition.x += tObject->mVelocity.x;
  tObject->mPosition.y += tObject->mVelocity.y;
  tObject->mPosition.z += tObject->mVelocity.z;

  double f = getFramerateFactor();
  tObject->mVelocity.x += tObject->mAcceleration.x*f;
  tObject->mVelocity.y += tObject->mAcceleration.y*f;
  tObject->mVelocity.z += tObject->mAcceleration.z*f;

  tObject->mVelocity.x *= gData.mOneMinusDragCoefficient.x;
  tObject->mVelocity.y *= gData.mOneMinusDragCoefficient.y;
  tObject->mVelocity.z *= gData.mOneMinusDragCoefficient.z;

  //TODO: fix velocity increase problem for 50Hz
  tObject->mVelocity.x = fmax(-gData.mMaxVelocity.x*f, fmin(gData.mMaxVelocity.x*f, tObject->mVelocity.x));
  tObject->mVelocity.y = fmax(-gData.mMaxVelocity.y, fmin(gData.mMaxVelocity.y, tObject->mVelocity.y));
  tObject->mVelocity.z = fmax(-gData.mMaxVelocity.x, fmin(gData.mMaxVelocity.z, tObject->mVelocity.z));

  tObject->mAcceleration.x = gData.mGravity.x;
  tObject->mAcceleration.y = gData.mGravity.y;
  tObject->mAcceleration.z = gData.mGravity.z;
}

void resetPhysicsObject(PhysicsObject* tObject) {
  memset(tObject, 0, sizeof(*tObject));
}

void resetPhysics() {
  resetMaxVelocity();
  resetDragCoefficient();

  Gravity gravity;
  gravity.x = 0;
  gravity.y = 0;
  gravity.z = 0;
  setGravity(gravity);

  gData.mIsPaused = 0;
}

void initPhysics() {
  resetPhysics();
}

void pausePhysics() {
	gData.mIsPaused = 1;
}
void resumePhysics() {
	gData.mIsPaused = 0;
}

int isEmptyVelocity(Velocity tVelocity) {
  return tVelocity.x == 0 && tVelocity.y == 0 && tVelocity.z == 0;
}
Velocity normalizeVelocity(Velocity tVelocity) {
	return vecNormalize(tVelocity);
}


