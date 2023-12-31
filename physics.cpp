#include "prism/physics.h"

#include <string.h>
#include <math.h>

#include "prism/log.h"
#include "prism/system.h"

static struct {
	Velocity mMaxVelocity;
	double mMaxVelocityDouble = INFINITY;
	Gravity mGravity;
	int mIsPaused;
	Vector3D mOneMinusDragCoefficient;
} gPrismPhysicsData;

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"

void imguiPhysics()
{
	static bool isWindowShown = false;
	imguiPrismAddTab("Prism", "Physics", &isWindowShown);
	if (isWindowShown)
	{
		ImGui::Begin("Physics", &isWindowShown);
		ImGui::Text("MaxVelocity = %f %f %f", gPrismPhysicsData.mMaxVelocity.x, gPrismPhysicsData.mMaxVelocity.y, gPrismPhysicsData.mMaxVelocity.z);
		ImGui::Text("MaxVelocityDouble = %f", gPrismPhysicsData.mMaxVelocityDouble);
		ImGui::Text("Gravity = %f %f %f", gPrismPhysicsData.mGravity.x, gPrismPhysicsData.mGravity.y, gPrismPhysicsData.mGravity.z);
		ImGui::Text("IsPaused = %d", gPrismPhysicsData.mIsPaused);
		ImGui::Text("OneMinusDragCoefficient = %f %f %f", gPrismPhysicsData.mOneMinusDragCoefficient.x, gPrismPhysicsData.mOneMinusDragCoefficient.y, gPrismPhysicsData.mOneMinusDragCoefficient.z);
		ImGui::End();
	}
}
#endif

void setMaxVelocity(const Velocity& tVelocity) {
  gPrismPhysicsData.mMaxVelocity = tVelocity;
}

void setMaxVelocityDouble(double tVelocity) {
   gPrismPhysicsData.mMaxVelocityDouble = tVelocity;
}

void resetMaxVelocity() {
  Velocity maxVelocity;
  maxVelocity.x = INFINITY;
  maxVelocity.y = INFINITY;
  maxVelocity.z = INFINITY;
  setMaxVelocity(maxVelocity);
  setMaxVelocityDouble(INFINITY);
}

void setDragCoefficient(const Vector3D& tDragCoefficient) {
  gPrismPhysicsData.mOneMinusDragCoefficient = vecAdd(Vector3D(1, 1, 1), vecScale(tDragCoefficient, -1));
}

void resetDragCoefficient() {
	gPrismPhysicsData.mOneMinusDragCoefficient = Vector3D(1,1,1);
}

void setGravity(const Gravity& tGravity) {
  gPrismPhysicsData.mGravity = tGravity;
}

Gravity getGravity() {
	return gPrismPhysicsData.mGravity;
}

static double f_min(double a, double b) {
  return (a < b) ? a : b;
}

static double f_max(double a, double b) {
  return (a > b) ? a : b;
}

void handlePhysics(PhysicsObject* tObject) {
  if(gPrismPhysicsData.mIsPaused) return;
  tObject->mPosition.x += tObject->mVelocity.x;
  tObject->mPosition.y += tObject->mVelocity.y;
  tObject->mPosition.z += tObject->mVelocity.z;

  tObject->mVelocity.x *= gPrismPhysicsData.mOneMinusDragCoefficient.x;
  tObject->mVelocity.y *= gPrismPhysicsData.mOneMinusDragCoefficient.y;
  tObject->mVelocity.z *= gPrismPhysicsData.mOneMinusDragCoefficient.z;

  double f = getFramerateFactor();
  tObject->mVelocity.x += tObject->mAcceleration.x*f;
  tObject->mVelocity.y += tObject->mAcceleration.y*f;
  tObject->mVelocity.z += tObject->mAcceleration.z*f;

  tObject->mVelocity.x = f_max(-gPrismPhysicsData.mMaxVelocity.x*f, f_min(gPrismPhysicsData.mMaxVelocity.x*f, tObject->mVelocity.x));
  tObject->mVelocity.y = f_max(-gPrismPhysicsData.mMaxVelocity.y, f_min(gPrismPhysicsData.mMaxVelocity.y, tObject->mVelocity.y));
  tObject->mVelocity.z = f_max(-gPrismPhysicsData.mMaxVelocity.x, f_min(gPrismPhysicsData.mMaxVelocity.z, tObject->mVelocity.z));
  if (vecLength(tObject->mVelocity) > gPrismPhysicsData.mMaxVelocityDouble) {
    tObject->mVelocity = vecNormalize(tObject->mVelocity) * gPrismPhysicsData.mMaxVelocityDouble * f;
  }

  tObject->mAcceleration.x = gPrismPhysicsData.mGravity.x;
  tObject->mAcceleration.y = gPrismPhysicsData.mGravity.y;
  tObject->mAcceleration.z = gPrismPhysicsData.mGravity.z;
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

  gPrismPhysicsData.mIsPaused = 0;
}

void initPhysics() {
  resetPhysics();
}

void pausePhysics() {
	gPrismPhysicsData.mIsPaused = 1;
}
void resumePhysics() {
	gPrismPhysicsData.mIsPaused = 0;
}

int isEmptyVelocity(const Velocity& tVelocity) {
  return tVelocity.x == 0 && tVelocity.y == 0 && tVelocity.z == 0;
}
Velocity normalizeVelocity(const Velocity& tVelocity) {
	return vecNormalize(tVelocity);
}


