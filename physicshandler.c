#include "prism/physicshandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "prism/datastructures.h"
#include "prism/physics.h"
#include "prism/memoryhandler.h"

typedef struct {

	PhysicsObject mObj;
	double mMaxVelocity;
	Vector3D mDragCoefficient;
	Gravity mGravity;

} HandledPhysicsData;

static struct {

	int mIsActive;
	IntMap mList;

} gData;

void setupPhysicsHandler() {
	if(gData.mIsActive) shutdownPhysicsHandler();
	gData.mIsActive = 1;
	gData.mList = new_int_map();
}

void shutdownPhysicsHandler() {
	if (!gData.mIsActive) return;
	int_map_empty(&gData.mList);
	gData.mIsActive = 0;

}

static void handleSinglePhysicsObjectInList(void* tCaller, void* tData) {
	(void) tCaller;
	HandledPhysicsData* data = tData;
	
	Gravity prevGravity = getGravity();	
	setGravity(data->mGravity);
	setMaxVelocityDouble(data->mMaxVelocity);
	setDragCoefficient(data->mDragCoefficient);
	handlePhysics(&data->mObj);
	resetMaxVelocity();
	resetDragCoefficient();
	setGravity(prevGravity);
}

void updatePhysicsHandler() {
	int_map_map(&gData.mList, handleSinglePhysicsObjectInList, NULL);
}

int addToPhysicsHandler(Position tPosition) {
	HandledPhysicsData* data = allocMemory(sizeof(HandledPhysicsData));
	resetPhysicsObject(&data->mObj);
	data->mObj.mPosition = tPosition;
	data->mMaxVelocity = INFINITY;
	data->mDragCoefficient = makePosition(0,0,0);
	data->mGravity = getGravity();

	return int_map_push_back_owned(&gData.mList, data);
}

void removeFromPhysicsHandler(int tID) {
	int_map_remove(&gData.mList, tID);
}

PhysicsObject* getPhysicsFromHandler(int tID) {
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	return &data->mObj;
}

Position* getHandledPhysicsPositionReference(int tID) {
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	return &data->mObj.mPosition;
}

Velocity * getHandledPhysicsVelocityReference(int tID)
{
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	return &data->mObj.mVelocity;
}

Acceleration* getHandledPhysicsAccelerationReference(int tID)
{
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	return &data->mObj.mAcceleration;
}

void addAccelerationToHandledPhysics(int tID, Acceleration tAccel) {
	PhysicsObject* obj = getPhysicsFromHandler(tID);
	obj->mAcceleration = vecAdd(obj->mAcceleration, tAccel);
}

void stopHandledPhysics(int tID) {
	PhysicsObject* obj = getPhysicsFromHandler(tID);
	obj->mVelocity = makePosition(0, 0, 0);
}

void setHandledPhysicsMaxVelocity(int tID, double tVelocity) {
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	data->mMaxVelocity = tVelocity;
}

void setHandledPhysicsDragCoefficient(int tID, Vector3D tDragCoefficient) {
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	data->mDragCoefficient = tDragCoefficient;
}

void setHandledPhysicsGravity(int tID, Vector3D tGravity) {
	HandledPhysicsData* data = int_map_get(&gData.mList, tID);
	data->mGravity = tGravity;
}


