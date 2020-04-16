#include "prism/actorhandler.h"

#include "prism/datastructures.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/wrapper.h"

typedef struct {
	ActorBlueprint mBP;
	ListIterator mListEntry;

	int mID;
	void* mData;
	int mIsOwned;
	int mIsPausable;
} Actor;

static struct {
	IntMap mActors;
	List mSequentialActorList;
	int mIsInitialized;
} gPrismActorHandlerData;

ActorBlueprint makeActorBlueprint(LoadActorFunction tLoad, UnloadActorFunction tUnload, UpdateActorFunction tUpdate, DrawActorFunction tDraw, IsActorActiveFunction tIsActive) {
	ActorBlueprint ret;
	ret.mLoad = tLoad;
	ret.mUnload = tUnload;
	ret.mUpdate = tUpdate;
	ret.mDraw = tDraw;
	ret.mIsActive = tIsActive;
	return ret;
}

void setupActorHandler()
{
	setProfilingSectionMarkerCurrentFunction();
	if (gPrismActorHandlerData.mIsInitialized) {
		logWarning("Actor handling already initialized.");
		shutdownActorHandler();
	}

	gPrismActorHandlerData.mActors = new_int_map();
	gPrismActorHandlerData.mSequentialActorList = new_list();
	gPrismActorHandlerData.mIsInitialized = 1;
}

static void unloadActor(Actor* e) {
	if(e->mBP.mUnload) e->mBP.mUnload(e->mData);

	if (e->mIsOwned) {
		freeMemory(e->mData);
	}

	int_map_remove(&gPrismActorHandlerData.mActors, e->mID);
}

static int removeActorCB(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = (Actor*)tData;
	unloadActor(e);
	return 1;
}

void shutdownActorHandler()
{
	setProfilingSectionMarkerCurrentFunction();
	list_remove_predicate_inverted(&gPrismActorHandlerData.mSequentialActorList, removeActorCB, NULL);
	delete_int_map(&gPrismActorHandlerData.mActors);
	delete_list(&gPrismActorHandlerData.mSequentialActorList);
	gPrismActorHandlerData.mIsInitialized = 0;
}

static int updateSingleActor(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = (Actor*)tData;
	if (isWrapperPaused() && e->mIsPausable) return 0;

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
	setProfilingSectionMarkerCurrentFunction();
	list_remove_predicate(&gPrismActorHandlerData.mSequentialActorList, updateSingleActor, NULL);
}

static void drawSingleActor(void* tCaller, void* tData) {
	(void)tCaller;
	Actor* e = (Actor*)tData;

	if(e->mBP.mDraw) e->mBP.mDraw(e->mData);
}

void drawActorHandler()
{
	setProfilingSectionMarkerCurrentFunction();
	list_map(&gPrismActorHandlerData.mSequentialActorList, drawSingleActor, NULL);
}

int instantiateActor(ActorBlueprint tBP)
{
	return instantiateActorWithData(tBP, NULL, 0);
}

int instantiateActorWithData(ActorBlueprint tBP, void * tData, int tIsOwned)
{
	Actor* e = (Actor*)allocMemory(sizeof(Actor));
	e->mBP = tBP;
	e->mData = tData;
	e->mIsOwned = tIsOwned;
	e->mIsPausable = 1;
	
	if (e->mBP.mLoad) e->mBP.mLoad(e->mData);

	list_push_back_owned(&gPrismActorHandlerData.mSequentialActorList, e);
	e->mListEntry = list_iterator_end(&gPrismActorHandlerData.mSequentialActorList);

	e->mID = int_map_push_back(&gPrismActorHandlerData.mActors, e);
	return e->mID;
}

void performOnActor(int tID, ActorInteractionFunction tFunc, void * tCaller)
{
	Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
	tFunc(tCaller, e->mData);
}

void setActorUnpausable(int tID)
{
	Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
	e->mIsPausable = 0;
}

void removeActor(int tID)
{
	Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
	unloadActor(e);
	list_iterator_remove(&gPrismActorHandlerData.mSequentialActorList, e->mListEntry);
}

void * getActorData(int tID)
{
	Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
	return e->mData;
}
