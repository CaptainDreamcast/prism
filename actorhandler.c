#include "tari/actorhandler.h"

#include "tari/datastructures.h"
#include "tari/log.h"
#include "tari/memoryhandler.h"

typedef struct {
	ActorBlueprint mBP;
	
	void* mData;
	int mIsOwned;

} Actor;

static struct {
	IntMap mActors;
	int mIsInitialized;
} gData;

void setupActorHandler()
{
	if (gData.mIsInitialized) {
		logWarning("Actor handling already initialized.");
		shutdownActorHandler();
	}

	gData.mActors = new_int_map();
	gData.mIsInitialized = 1;
}

static void unloadActor(Actor* e) {
	if(e->mBP.mUnload) e->mBP.mUnload(e->mData);

	if (e->mIsOwned) {
		freeMemory(e->mData);
	}
}

static int removeActorCB(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = tData;
	unloadActor(e);
	return 1;
}

void shutdownActorHandler()
{
	int_map_remove_predicate(&gData.mActors, removeActorCB, NULL);
	delete_int_map(&gData.mActors);
	gData.mIsInitialized = 0;
}

static int updateSingleActor(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = tData;

	int isActive = e->mBP.mIsActive == NULL || e->mBP.mIsActive(e->mData);
	if (!isActive) {
		unloadActor(e);
		return 1;
	}

	if(e->mBP.mUpdate) e->mBP.mUpdate(e->mData);
	return 0;
}

void updateActorHandler()
{
	int_map_remove_predicate(&gData.mActors, updateSingleActor, NULL);
}

static void drawSingleActor(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = tData;

	if(e->mBP.mDraw) e->mBP.mDraw(e->mData);
}

void drawActorHandler()
{
	int_map_map(&gData.mActors, drawSingleActor, NULL);
}

int instantiateActor(ActorBlueprint tBP)
{
	return instantiateActorWithData(tBP, NULL, 0);
}

int instantiateActorWithData(ActorBlueprint tBP, void * tData, int tIsOwned)
{
	Actor* e = allocMemory(sizeof(Actor));
	e->mBP = tBP;
	e->mData = tData;
	e->mIsOwned = tIsOwned;
	
	if (e->mBP.mLoad) e->mBP.mLoad(e->mData);

	return int_map_push_back_owned(&gData.mActors, e);
}

void performOnActor(int tID, ActorInteractionFunction tFunc, void * tCaller)
{
	Actor* e = int_map_get(&gData.mActors, tID);
	tFunc(tCaller, e->mData);
}

void removeActor(int tID)
{
	Actor* e = int_map_get(&gData.mActors, tID);
	unloadActor(e);
	int_map_remove(&gData.mActors, tID);
}
