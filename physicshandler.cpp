#include "prism/physicshandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <map>

#include "prism/datastructures.h"
#include "prism/physics.h"
#include "prism/memoryhandler.h"
#include "prism/stlutil.h"

using namespace std;

static struct {
	int mIsActive;
	map<int, PhysicsHandlerElement> mList;
} gPhysicsHandler;

void setupPhysicsHandler() {
	if(gPhysicsHandler.mIsActive) shutdownPhysicsHandler();
	gPhysicsHandler.mIsActive = 1;
	gPhysicsHandler.mList.clear();
}

void shutdownPhysicsHandler() {
	if (!gPhysicsHandler.mIsActive) return;
	stl_delete_map(gPhysicsHandler.mList);
	gPhysicsHandler.mIsActive = 0;

}

static void handleSinglePhysicsObjectInList(void* tCaller, PhysicsHandlerElement &tData) {
	(void) tCaller;
	PhysicsHandlerElement* data = &tData;
	if (data->mIsPaused) return;
	data->mTimeDilatationNow += data->mTimeDilatation;
	int updateAmount = (int)data->mTimeDilatationNow;
	data->mTimeDilatationNow -= updateAmount;
	while (updateAmount--) {
		Gravity prevGravity = getGravity();
		setGravity(data->mGravity);
		setMaxVelocityDouble(data->mMaxVelocity);
		setDragCoefficient(data->mDragCoefficient);
		handlePhysics(&data->mObj);
		resetMaxVelocity();
		resetDragCoefficient();
		setGravity(prevGravity);
	}
}

void updatePhysicsHandler() {
	stl_int_map_map(gPhysicsHandler.mList, handleSinglePhysicsObjectInList);
}

PhysicsHandlerElement* addToPhysicsHandler(const Position& tPosition) {
	PhysicsHandlerElement data;
	resetPhysicsObject(&data.mObj);
	data.mObj.mPosition = tPosition;
	data.mMaxVelocity = INFINITY;
	data.mDragCoefficient = Vector3D(0,0,0);
	data.mGravity = getGravity();
	data.mIsPaused = 0;
	data.mTimeDilatation = 1.0;
	data.mTimeDilatationNow = 0.0;

	const auto id = stl_int_map_push_back(gPhysicsHandler.mList, data);
	auto& element = gPhysicsHandler.mList[id];
	element.mID = id;
	return &element;
}

void removeFromPhysicsHandler(PhysicsHandlerElement* data) {
	gPhysicsHandler.mList.erase(data->mID);
}

PhysicsObject* getPhysicsFromHandler(PhysicsHandlerElement* data) {
	return &data->mObj;
}

Position getHandledPhysicsPosition(PhysicsHandlerElement* data)
{
	return data->mObj.mPosition;
}

Position* getHandledPhysicsPositionReference(PhysicsHandlerElement* data) {
	return &data->mObj.mPosition;
}

Velocity * getHandledPhysicsVelocityReference(PhysicsHandlerElement* data)
{
	return &data->mObj.mVelocity;
}

Acceleration* getHandledPhysicsAccelerationReference(PhysicsHandlerElement* data)
{
	return &data->mObj.mAcceleration;
}

void addAccelerationToHandledPhysics(PhysicsHandlerElement* data, const Acceleration& tAccel) {
	if (data->mIsPaused) return;
	PhysicsObject* obj = &data->mObj;
	obj->mAcceleration = vecAdd(obj->mAcceleration, tAccel);
}

void stopHandledPhysics(PhysicsHandlerElement* data) {
	PhysicsObject* obj = &data->mObj;
	obj->mVelocity = Vector3D(0, 0, 0);
}

void pauseHandledPhysics(PhysicsHandlerElement* data)
{
	data->mIsPaused = 1;
}

void resumeHandledPhysics(PhysicsHandlerElement* data)
{
	data->mIsPaused = 0;
}

void setHandledPhysicsMaxVelocity(PhysicsHandlerElement* data, double tVelocity) {
	data->mMaxVelocity = tVelocity;
}

void setHandledPhysicsDragCoefficient(PhysicsHandlerElement* data, const Vector3D& tDragCoefficient) {
	data->mDragCoefficient = tDragCoefficient;
}

void setHandledPhysicsGravity(PhysicsHandlerElement* data, const Vector3D& tGravity) {
	data->mGravity = tGravity;
}

void setHandledPhysicsSpeed(PhysicsHandlerElement* data, double tSpeed) {
	data->mTimeDilatation = tSpeed;
}