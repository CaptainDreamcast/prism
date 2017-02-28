#include "include/physicshandler.h"

#include <stdlib.h>
#include <math.h>

#include "include/datastructures.h"
#include "include/physics.h"
#include "include/memoryhandler.h"

typedef struct {

	PhysicsObject mObj;
	double mMaxVelocity;
	Vector3D mDragCoefficient;
	Gravity mGravity;

} HandledPhysicsData;

static struct {

	int mIsActive;
	List mList;

} gData;

void setupPhysicsHandler() {
	if(gData.mIsActive) shutdownPhysicsHandler();
	gData.mIsActive = 1;
	gData.mList = new_list();
}

void shutdownPhysicsHandler() {
	list_empty(&gData.mList);
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
	list_map(&gData.mList, handleSinglePhysicsObjectInList, NULL);
}

int addToPhysicsHandler(Position tPosition) {
	HandledPhysicsData* data = allocMemory(sizeof(HandledPhysicsData));
	resetPhysicsObject(&data->mObj);
	data->mObj.mPosition = tPosition;
	data->mMaxVelocity = INFINITY;
	data->mDragCoefficient = makePosition(0,0,0);
	data->mGravity = getGravity();

	return list_push_front_owned(&gData.mList, data);
}

void removeFromPhysicsHandler(int tID) {
	list_remove(&gData.mList, tID);
}

PhysicsObject* getPhysicsFromHandler(int tID) {
	HandledPhysicsData* data = list_get(&gData.mList, tID);
	return &data->mObj;
}

void addAccelerationToHandledPhysics(int tID, Acceleration tAccel) {
	PhysicsObject* obj = getPhysicsFromHandler(tID);
	obj->mAcceleration = vecAdd(obj->mAcceleration, tAccel);
}

void setHandledPhysicsMaxVelocity(int tID, double tVelocity) {
	HandledPhysicsData* data = list_get(&gData.mList, tID);
	data->mMaxVelocity = tVelocity;
}

void setHandledPhysicsDragCoefficient(int tID, Vector3D tDragCoefficient) {
	HandledPhysicsData* data = list_get(&gData.mList, tID);
	data->mDragCoefficient = tDragCoefficient;
}

void setHandledPhysicsGravity(int tID, Vector3D tGravity) {
	HandledPhysicsData* data = list_get(&gData.mList, tID);
	data->mGravity = tGravity;
}
