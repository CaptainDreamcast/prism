#include "prism/actorhandler.h"

#include "prism/datastructures.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"

typedef struct {
	ActorBlueprint mBP;
	ListIterator mListEntry;

	int mID;
	void* mData;
	int mIsOwned;
} Actor;

static struct {
	IntMap mActors;
	List mSequentialActorList;
	int mIsInitialized;
} gData;

void setupActorHandler()
{
	if (gData.mIsInitialized) {
		logWarning("Actor handling already initialized.");
		shutdownActorHandler();
	}

	gData.mActors = new_int_map();
	gData.mSequentialActorList = new_list();
	gData.mIsInitialized = 1;
}

static void unloadActor(Actor* e) {
	if(e->mBP.mUnload) e->mBP.mUnload(e->mData);

	if (e->mIsOwned) {
		freeMemory(e->mData);
	}

	int_map_remove(&gData.mActors, e->mID);
}

static int removeActorCB(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = tData;
	unloadActor(e);
	return 1;
}

void shutdownActorHandler()
{
	list_remove_predicate(&gData.mSequentialActorList, removeActorCB, NULL);
	delete_int_map(&gData.mActors);
	delete_list(&gData.mSequentialActorList);
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
	list_remove_predicate(&gData.mSequentialActorList, updateSingleActor, NULL);
}

static void drawSingleActor(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = tData;

	if(e->mBP.mDraw) e->mBP.mDraw(e->mData);
}

void drawActorHandler()
{
	list_map(&gData.mSequentialActorList, drawSingleActor, NULL);
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

	list_push_back_owned(&gData.mSequentialActorList, e);
	e->mListEntry = list_iterator_end(&gData.mSequentialActorList);

	e->mID = int_map_push_back(&gData.mActors, e);
	return e->mID;
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
	list_iterator_remove(&gData.mSequentialActorList, e->mListEntry);
}

void * getActorData(int tID)
{
	Actor* e = int_map_get(&gData.mActors, tID);
	return e->mData;
}
