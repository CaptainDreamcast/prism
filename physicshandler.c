#include "include/physicshandler.h"

#include <stdlib.h>

#include "include/datastructures.h"
#include "include/physics.h"
#include "include/memoryhandler.h"

typedef struct {

	PhysicsObject mObj;

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
	
	handlePhysics(&data->mObj);
}

void updatePhysicsHandler() {
	list_map(&gData.mList, handleSinglePhysicsObjectInList, NULL);
}

int addToPhysicsHandler(Position tPosition) {
	HandledPhysicsData* data = allocMemory(sizeof(HandledPhysicsData));
	resetPhysicsObject(&data->mObj);
	data->mObj.mPosition = tPosition;

	return list_push_front_owned(&gData.mList, data);
}

PhysicsObject* getPhysicsFromHandler(int tID) {
	HandledPhysicsData* data = list_get(&gData.mList, tID);
	return &data->mObj;
}
