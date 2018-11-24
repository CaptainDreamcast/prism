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

typedef struct {

	PhysicsObject mObj;
	double mMaxVelocity;
	Vector3D mDragCoefficient;
	Gravity mGravity;
	int mIsPaused;
} HandledPhysicsData;

static struct {

	int mIsActive;
	map<int, HandledPhysicsData> mList;

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

static void handleSinglePhysicsObjectInList(void* tCaller, HandledPhysicsData &tData) {
	(void) tCaller;
	HandledPhysicsData* data = &tData;
	if (data->mIsPaused) return;

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
	stl_int_map_map(gPhysicsHandler.mList, handleSinglePhysicsObjectInList);
}

int addToPhysicsHandler(Position tPosition) {
	HandledPhysicsData data;
	resetPhysicsObject(&data.mObj);
	data.mObj.mPosition = tPosition;
	data.mMaxVelocity = INFINITY;
	data.mDragCoefficient = makePosition(0,0,0);
	data.mGravity = getGravity();
	data.mIsPaused = 0;

	return stl_int_map_push_back(gPhysicsHandler.mList, data);
}

void removeFromPhysicsHandler(int tID) {
	gPhysicsHandler.mList.erase(tID);
}

PhysicsObject* getPhysicsFromHandler(int tID) {
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	return &data->mObj;
}

Position* getHandledPhysicsPositionReference(int tID) {
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	return &data->mObj.mPosition;
}

Velocity * getHandledPhysicsVelocityReference(int tID)
{
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	return &data->mObj.mVelocity;
}

Acceleration* getHandledPhysicsAccelerationReference(int tID)
{
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	return &data->mObj.mAcceleration;
}

void addAccelerationToHandledPhysics(int tID, Acceleration tAccel) {
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	if (data->mIsPaused) return;
	PhysicsObject* obj = &data->mObj;
	obj->mAcceleration = vecAdd(obj->mAcceleration, tAccel);
}

void stopHandledPhysics(int tID) {
	PhysicsObject* obj = getPhysicsFromHandler(tID);
	obj->mVelocity = makePosition(0, 0, 0);
}

void pauseHandledPhysics(int tID)
{
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	data->mIsPaused = 1;
}

void resumeHandledPhysics(int tID)
{
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	data->mIsPaused = 0;
}

void setHandledPhysicsMaxVelocity(int tID, double tVelocity) {
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	data->mMaxVelocity = tVelocity;
}

void setHandledPhysicsDragCoefficient(int tID, Vector3D tDragCoefficient) {
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	data->mDragCoefficient = tDragCoefficient;
}

void setHandledPhysicsGravity(int tID, Vector3D tGravity) {
	HandledPhysicsData* data = &gPhysicsHandler.mList[tID];
	data->mGravity = tGravity;
}


