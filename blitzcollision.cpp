#include "prism/blitzcollision.h"

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/blitzentity.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/collisionhandler.h"
#include "prism/blitzmugenanimation.h"
#include "prism/blitzphysics.h"
#include "prism/stlutil.h"

using namespace std;

typedef struct {
	void(*mFunc)(void*, void*);
	void* mCaller;
} BlitzCollisionCallbackData;

typedef struct {
	int mEntityID;
	int mCollisionListID;

	int mOwnsCollisionHandlerObject;
	int mCollisionHandlerID;

	void* mCollisionData;

	int mIsSolid;
	int mIsMovable;

	List mCollisionCallbacks;
} BlitzCollisionObject;

typedef struct {
	BlitzCollisionObject* a;
	BlitzCollisionObject* b;
} ActiveSolidCollision;


typedef struct {
	int mEntityID;

	int mIsTopCollided;
	int mIsBottomCollided;
	int mIsLeftCollided;
	int mIsRightCollided;

	map<int, BlitzCollisionObject> mCollisionObjects;
	int mIsEmpty;
} CollisionEntry;

static struct {
	map<int, CollisionEntry> mEntries;
	list<ActiveSolidCollision> mActiveSolidCollisions;
} gBlitzCollisionData;

static void loadBlitzCollisionHandler(void* tData) {
	(void)tData;
	gBlitzCollisionData.mEntries.clear();
	gBlitzCollisionData.mActiveSolidCollisions.clear();
}

static void unloadBlitzCollisionHandler(void* tData) {
	(void)tData;
	gBlitzCollisionData.mEntries.clear();
	gBlitzCollisionData.mActiveSolidCollisions.clear();
}

// TODO: fix after proper sequential actor handler is implemented
static void updateSingleBlitzCollidedValue(int* tValue) {
	*tValue = 0;
}

static void updateSingleBlitzCollisionEntry(void* , CollisionEntry& tData) {
	CollisionEntry* e = &tData;
	updateSingleBlitzCollidedValue(&e->mIsTopCollided);
	updateSingleBlitzCollidedValue(&e->mIsBottomCollided);
	updateSingleBlitzCollidedValue(&e->mIsLeftCollided);
	updateSingleBlitzCollidedValue(&e->mIsRightCollided);
}

static CollisionEntry* getBlitzCollisionEntry(int tEntityID) {
	if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have collision component.", tEntityID);
		recoverFromError();
	}

	return &gBlitzCollisionData.mEntries[tEntityID];
}

static BlitzCollisionObject* getBlitzCollisionObject(int tEntityID, int tCollisionID) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);

	if (!stl_map_contains(e->mCollisionObjects, tCollisionID)) {
		logErrorFormat("Entity with ID %d does not have collision with id %d.", tEntityID, tCollisionID);
		recoverFromError();
	}

	return &e->mCollisionObjects[tCollisionID];
}

static void setBlitzCollisionTopCollided(int tEntityID, int tIsCollided) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	e->mIsTopCollided |= tIsCollided;
}

static void setBlitzCollisionBottomCollided(int tEntityID, int tIsCollided) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	e->mIsBottomCollided |= tIsCollided;
}

static void setBlitzCollisionLeftCollided(int tEntityID, int tIsCollided) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	e->mIsLeftCollided |= tIsCollided;
}

static void setBlitzCollisionRightCollided(int tEntityID, int tIsCollided) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	e->mIsRightCollided |= tIsCollided;
}


static int updateSingleSolidCollision(void* , ActiveSolidCollision& tData) {
	ActiveSolidCollision* e = &tData;
	BlitzCollisionObject* selfObject = e->a;
	BlitzCollisionObject* otherObject = e->b;
	if (!isHandledCollisionValid(selfObject->mCollisionListID, selfObject->mCollisionHandlerID)) return 1;
	if (!isHandledCollisionValid(otherObject->mCollisionListID, otherObject->mCollisionHandlerID)) return 1;

	if (selfObject->mIsSolid && otherObject->mIsSolid) {
		if (selfObject->mIsMovable && !otherObject->mIsMovable) {
			resolveHandledCollisionMovableStatic(selfObject->mCollisionListID, selfObject->mCollisionHandlerID, otherObject->mCollisionListID, otherObject->mCollisionHandlerID, getBlitzEntityPositionReference(selfObject->mEntityID), getBlitzPhysicsVelocity(selfObject->mEntityID));
			setBlitzCollisionTopCollided(selfObject->mEntityID, isHandledCollisionBelowOtherCollision(selfObject->mCollisionListID, selfObject->mCollisionHandlerID, otherObject->mCollisionListID, otherObject->mCollisionHandlerID));
			setBlitzCollisionBottomCollided(selfObject->mEntityID, isHandledCollisionAboveOtherCollision(selfObject->mCollisionListID, selfObject->mCollisionHandlerID, otherObject->mCollisionListID, otherObject->mCollisionHandlerID));
			setBlitzCollisionLeftCollided(selfObject->mEntityID, isHandledCollisionRightOfOtherCollision(selfObject->mCollisionListID, selfObject->mCollisionHandlerID, otherObject->mCollisionListID, otherObject->mCollisionHandlerID));
			setBlitzCollisionRightCollided(selfObject->mEntityID, isHandledCollisionLeftOfOtherCollision(selfObject->mCollisionListID, selfObject->mCollisionHandlerID, otherObject->mCollisionListID, otherObject->mCollisionHandlerID));
		}
	}
	return 1;
}

static void updateBlitzCollisionHandler(void* tData) {
	(void)tData;
	
	stl_int_map_map(gBlitzCollisionData.mEntries, updateSingleBlitzCollisionEntry);
	stl_list_remove_predicate(gBlitzCollisionData.mActiveSolidCollisions, updateSingleSolidCollision);
}

static void unregisterEntity(int tEntityID);

static BlitzComponent getBlitzCollisionComponent() {
	return makeBlitzComponent(unregisterEntity);
}

ActorBlueprint getBlitzCollisionHandler(){
	return makeActorBlueprint(loadBlitzCollisionHandler, unloadBlitzCollisionHandler, updateBlitzCollisionHandler);
}

static void internalCollisionHandleSingleCB(void* tCaller, void* tData) {
	BlitzCollisionObject* otherObject = (BlitzCollisionObject*)tCaller;
	BlitzCollisionCallbackData* callbackData = (BlitzCollisionCallbackData*)tData;

	callbackData->mFunc(callbackData->mCaller, otherObject->mCollisionData);
}

static void internalCollisionCB(void* tCaller, void* tCollisionData) {
	BlitzCollisionObject* selfObject = (BlitzCollisionObject*)tCaller;
	BlitzCollisionObject* otherObject = (BlitzCollisionObject*)tCollisionData;

	if (selfObject->mIsSolid && otherObject->mIsSolid) {
		if (selfObject->mIsMovable && !otherObject->mIsMovable) {
			ActiveSolidCollision solidCollision;
			solidCollision.a = selfObject;
			solidCollision.b = otherObject;
			gBlitzCollisionData.mActiveSolidCollisions.push_back(solidCollision);
		}
	}

	list_map(&selfObject->mCollisionCallbacks, internalCollisionHandleSingleCB, otherObject);
}

void addBlitzCollisionComponent(int tEntityID)
{
	CollisionEntry e;
	e.mEntityID = tEntityID;
	registerBlitzComponent(tEntityID, getBlitzCollisionComponent());
	gBlitzCollisionData.mEntries[tEntityID] = e;
}

void removeBlitzCollisionComponent(int tEntityID)
{
	unregisterEntity(tEntityID);
}

static int addEmptyCollisionObject(CollisionEntry* tEntry, int tList, int tOwnsCollisionHandlerObject) {
	int id = stl_int_map_push_back(tEntry->mCollisionObjects, BlitzCollisionObject());
	BlitzCollisionObject* e = &tEntry->mCollisionObjects[id];
	e->mEntityID = tEntry->mEntityID;
	e->mCollisionListID = tList;
	e->mOwnsCollisionHandlerObject = tOwnsCollisionHandlerObject;
	e->mCollisionHandlerID = -1;
	e->mCollisionData = NULL;
	e->mIsSolid = 0;
	e->mCollisionCallbacks = new_list();
	return id;
}

int addBlitzCollisionPassiveMugen(int tEntityID, int tList)
{
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int mugenAnimationID = getBlitzMugenAnimationID(tEntityID);
	int collisionID = addEmptyCollisionObject(e, tList, 0);
	BlitzCollisionObject* object = &e->mCollisionObjects[collisionID];
	setMugenAnimationPassiveCollisionActive(mugenAnimationID, tList, internalCollisionCB, object, object);

	return collisionID;
}

int addBlitzCollisionAttackMugen(int tEntityID, int tList) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int mugenAnimationID = getBlitzMugenAnimationID(tEntityID);
	int collisionID = addEmptyCollisionObject(e, tList, 0);
	BlitzCollisionObject* object = &e->mCollisionObjects[collisionID];
	setMugenAnimationAttackCollisionActive(mugenAnimationID, tList, internalCollisionCB, object, object);
	
	return collisionID;
}

int addBlitzCollisionRect(int tEntityID, int tList, CollisionRect tRectangle)
{
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int collisionObjectID = addEmptyCollisionObject(e, tList, 1);
	BlitzCollisionObject* object = &e->mCollisionObjects[collisionObjectID];
	object->mCollisionHandlerID = addCollisionRectangleToCollisionHandler(tList, getBlitzEntityPositionReference(tEntityID), tRectangle, internalCollisionCB, object, object);

	return collisionObjectID;
}

void changeBlitzCollisionRect(int tEntityID, int tCollisionID, CollisionRect tRectangle) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	BlitzCollisionObject* object = &e->mCollisionObjects[tCollisionID];
	changeCollisionRectangleInCollisionHandler(object->mCollisionListID, object->mCollisionHandlerID, tRectangle);
}

int addBlitzCollisionCirc(int tEntityID, int tList, CollisionCirc tCircle)
{
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int collisionObjectID = addEmptyCollisionObject(e, tList, 1);
	BlitzCollisionObject* object = &e->mCollisionObjects[collisionObjectID];
	object->mCollisionHandlerID = addCollisionCircleToCollisionHandler(tList, getBlitzEntityPositionReference(tEntityID), tCircle, internalCollisionCB, object, object);

	return collisionObjectID;
}

void addBlitzCollisionCB(int tEntityID, int tCollisionID, void(*tCB)(void *, void*), void* tCaller)
{
	BlitzCollisionObject* object = getBlitzCollisionObject(tEntityID, tCollisionID);
	BlitzCollisionCallbackData* e = (BlitzCollisionCallbackData*)allocMemory(sizeof(BlitzCollisionCallbackData));
	e->mFunc = tCB;
	e->mCaller = tCaller;
	list_push_back_owned(&object->mCollisionCallbacks, e);
}

void setBlitzCollisionCollisionData(int tEntityID, int tCollisionID, void* tCollisionData) {
	BlitzCollisionObject* e = getBlitzCollisionObject(tEntityID, tCollisionID);
	e->mCollisionData = tCollisionData;
}

void setBlitzCollisionSolid(int tEntityID, int tCollisionID, int tIsMovable)
{
	BlitzCollisionObject* e = getBlitzCollisionObject(tEntityID, tCollisionID);
	if (!e->mOwnsCollisionHandlerObject) {
		logErrorFormat("Unable to set entity %d collision id %d solid, does not own collision handler entry.", tEntityID, tCollisionID);
		recoverFromError();
	}

	e->mIsMovable = tIsMovable;
	e->mIsSolid = 1;
}

int hasBlitzCollidedTop(int tEntityID)
{
	if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
	return e->mIsTopCollided;
}

int hasBlitzCollidedBottom(int tEntityID)
{
	if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
	return e->mIsBottomCollided;
}

int hasBlitzCollidedLeft(int tEntityID)
{
	if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
	return e->mIsLeftCollided;
}

int hasBlitzCollidedRight(int tEntityID)
{
	if (!stl_map_contains(gBlitzCollisionData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
	return e->mIsRightCollided;
}



static int removeSingleCollisionObject(void* , BlitzCollisionObject& tData) {
	BlitzCollisionObject* e = &tData;
	if (e->mOwnsCollisionHandlerObject) {
		removeFromCollisionHandler(e->mCollisionListID, e->mCollisionHandlerID);
	}
	delete_list(&e->mCollisionCallbacks);

	return 1;
}

static void unregisterEntity(int tEntityID) {
	CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
	stl_int_map_remove_predicate(e->mCollisionObjects, removeSingleCollisionObject);
	gBlitzCollisionData.mEntries.erase(tEntityID);
}

void removeAllBlitzCollisions(int tEntityID)
{
	CollisionEntry* e = &gBlitzCollisionData.mEntries[tEntityID];
	stl_int_map_remove_predicate(e->mCollisionObjects, removeSingleCollisionObject);
}
