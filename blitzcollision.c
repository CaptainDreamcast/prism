#include "prism/blitzcollision.h"

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/blitzentity.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/collisionhandler.h"
#include "prism/blitzmugenanimation.h"
#include "prism/blitzphysics.h"


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

	IntMap mCollisionObjects; // contains BlitzCollisionObjects
	int mIsEmpty;
} CollisionEntry;

static struct {
	IntMap mEntries;
	List mActiveSolidCollisions;
} gData;

static void loadBlitzCollisionHandler(void* tData) {
	(void)tData;
	gData.mEntries = new_int_map();
	gData.mActiveSolidCollisions = new_list();
}

// TODO: fix after proper sequential actor handler is implemented
static void updateSingleBlitzCollidedValue(int* tValue) {
	*tValue = 0;
}

static void updateSingleBlitzCollisionEntry(void* tCaller, void* tData) {
	(void)tCaller;
	CollisionEntry* e = tData;
	updateSingleBlitzCollidedValue(&e->mIsTopCollided);
	updateSingleBlitzCollidedValue(&e->mIsBottomCollided);
	updateSingleBlitzCollidedValue(&e->mIsLeftCollided);
	updateSingleBlitzCollidedValue(&e->mIsRightCollided);
}

static CollisionEntry* getBlitzCollisionEntry(int tEntityID) {
	if (!int_map_contains(&gData.mEntries, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have collision component.", tEntityID);
		recoverFromError();
	}

	return int_map_get(&gData.mEntries, tEntityID);
}

static BlitzCollisionObject* getBlitzCollisionObject(int tEntityID, int tCollisionID) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);

	if (!int_map_contains(&e->mCollisionObjects, tCollisionID)) {
		logErrorFormat("Entity with ID %d does not have collision with id %d.", tEntityID, tCollisionID);
		recoverFromError();
	}

	return int_map_get(&e->mCollisionObjects, tCollisionID);
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


static int updateSingleSolidCollision(void* tCaller, void* tData) {
	(void)tCaller;
	ActiveSolidCollision* e = tData;
	BlitzCollisionObject* selfObject = e->a;
	BlitzCollisionObject* otherObject = e->b;
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
	
	int_map_map(&gData.mEntries, updateSingleBlitzCollisionEntry, NULL);
	list_remove_predicate(&gData.mActiveSolidCollisions, updateSingleSolidCollision, NULL);
}

static void unregisterEntity(int tEntityID);

static BlitzComponent BlitzCollisionComponent = {
	.mUnregisterEntity = unregisterEntity,
};

ActorBlueprint BlitzCollisionHandler = {
	.mLoad = loadBlitzCollisionHandler,
	.mUpdate = updateBlitzCollisionHandler,
};

static void internalCollisionHandleSingleCB(void* tCaller, void* tData) {
	BlitzCollisionObject* otherObject = tCaller;
	BlitzCollisionCallbackData* callbackData = tData;

	callbackData->mFunc(callbackData->mCaller, otherObject->mCollisionData);
}

static void internalCollisionCB(void* tCaller, void* tCollisionData) {
	BlitzCollisionObject* selfObject = tCaller;
	BlitzCollisionObject* otherObject = tCollisionData;

	if (selfObject->mIsSolid && otherObject->mIsSolid) {
		if (selfObject->mIsMovable && !otherObject->mIsMovable) {
			ActiveSolidCollision* solidCollision = allocMemory(sizeof(ActiveSolidCollision));
			solidCollision->a = selfObject;
			solidCollision->b = otherObject;
			list_push_back_owned(&gData.mActiveSolidCollisions, solidCollision);
		}
	}

	list_map(&selfObject->mCollisionCallbacks, internalCollisionHandleSingleCB, otherObject);
}

void addBlitzCollisionComponent(int tEntityID)
{
	CollisionEntry* e = allocMemory(sizeof(CollisionEntry));
	e->mEntityID = tEntityID;
	
	e->mCollisionObjects = new_int_map();

	registerBlitzComponent(tEntityID, BlitzCollisionComponent);
	int_map_push_owned(&gData.mEntries, tEntityID, e);
}

void removeBlitzCollisionComponent(int tEntityID)
{
	unregisterEntity(tEntityID);
}

static int addEmptyCollisionObject(CollisionEntry* tEntry, int tList, int tOwnsCollisionHandlerObject) {
	BlitzCollisionObject* e = allocMemory(sizeof(BlitzCollisionObject));
	e->mEntityID = tEntry->mEntityID;
	e->mCollisionListID = tList;
	e->mOwnsCollisionHandlerObject = tOwnsCollisionHandlerObject;
	e->mCollisionHandlerID = -1;
	e->mCollisionData = NULL;
	e->mIsSolid = 0;
	e->mCollisionCallbacks = new_list();
	
	return int_map_push_back_owned(&tEntry->mCollisionObjects, e);
}

int addBlitzCollisionPassiveMugen(int tEntityID, int tList)
{
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int mugenAnimationID = getBlitzMugenAnimationID(tEntityID);
	int collisionID = addEmptyCollisionObject(e, tList, 0);
	BlitzCollisionObject* object = int_map_get(&e->mCollisionObjects, collisionID);
	setMugenAnimationPassiveCollisionActive(mugenAnimationID, tList, internalCollisionCB, object, object);

	return collisionID;
}

int addBlitzCollisionAttackMugen(int tEntityID, int tList) {
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int mugenAnimationID = getBlitzMugenAnimationID(tEntityID);
	int collisionID = addEmptyCollisionObject(e, tList, 0);
	BlitzCollisionObject* object = int_map_get(&e->mCollisionObjects, collisionID);
	setMugenAnimationAttackCollisionActive(mugenAnimationID, tList, internalCollisionCB, object, object);
	
	return collisionID;
}

int addBlitzCollisionRect(int tEntityID, int tList, CollisionRect tRectangle)
{
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int collisionObjectID = addEmptyCollisionObject(e, tList, 1);
	BlitzCollisionObject* object = int_map_get(&e->mCollisionObjects, collisionObjectID);
	object->mCollisionHandlerID = addCollisionRectangleToCollisionHandler(tList, getBlitzEntityPositionReference(tEntityID), tRectangle, internalCollisionCB, object, object);

	return collisionObjectID;
}

int addBlitzCollisionCirc(int tEntityID, int tList, CollisionCirc tCircle)
{
	CollisionEntry* e = getBlitzCollisionEntry(tEntityID);
	int collisionObjectID = addEmptyCollisionObject(e, tList, 1);
	BlitzCollisionObject* object = int_map_get(&e->mCollisionObjects, collisionObjectID);
	object->mCollisionHandlerID = addCollisionCircleToCollisionHandler(tList, getBlitzEntityPositionReference(tEntityID), tCircle, internalCollisionCB, object, object);

	return collisionObjectID;
}

void addBlitzCollisionCB(int tEntityID, int tCollisionID, void(*tCB)(void *, void*), void* tCaller)
{
	BlitzCollisionObject* object = getBlitzCollisionObject(tEntityID, tCollisionID);
	BlitzCollisionCallbackData* e = allocMemory(sizeof(BlitzCollisionCallbackData));
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
	if (!int_map_contains(&gData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = int_map_get(&gData.mEntries, tEntityID);
	return e->mIsTopCollided;
}

int hasBlitzCollidedBottom(int tEntityID)
{
	if (!int_map_contains(&gData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = int_map_get(&gData.mEntries, tEntityID);
	return e->mIsBottomCollided;
}

int hasBlitzCollidedLeft(int tEntityID)
{
	if (!int_map_contains(&gData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = int_map_get(&gData.mEntries, tEntityID);
	return e->mIsLeftCollided;
}

int hasBlitzCollidedRight(int tEntityID)
{
	if (!int_map_contains(&gData.mEntries, tEntityID)) return 0;

	CollisionEntry* e = int_map_get(&gData.mEntries, tEntityID);
	return e->mIsRightCollided;
}



static int removeSingleCollisionObject(void* tCaller, void* tData) {
	(void)tCaller;
	BlitzCollisionObject* e = tData;
	if (e->mOwnsCollisionHandlerObject) {
		removeFromCollisionHandler(e->mCollisionListID, e->mCollisionHandlerID);
	}
	delete_list(&e->mCollisionCallbacks);

	return 1;
}

static void unregisterEntity(int tEntityID) {
	CollisionEntry* e = int_map_get(&gData.mEntries, tEntityID);
	int_map_remove_predicate(&e->mCollisionObjects, removeSingleCollisionObject, NULL);
	delete_int_map(&e->mCollisionObjects);
	int_map_remove(&gData.mEntries, tEntityID);
}

void removeAllBlitzCollisions(int tEntityID)
{
	CollisionEntry* e = int_map_get(&gData.mEntries, tEntityID);
	int_map_remove_predicate(&e->mCollisionObjects, removeSingleCollisionObject, NULL);
}