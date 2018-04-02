#include "prism/blitzphysics.h"

#include "prism/datastructures.h"
#include "prism/physics.h"
#include "prism/memoryhandler.h"
#include "prism/blitzentity.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/blitzcollision.h"
#include "prism/math.h"

typedef struct {
	int mEntityID;

	Velocity mVelocity;
	Acceleration mAcceleration;
	Acceleration mGravity;
	Vector3D mOneMinusDragOnCollision;
} PhysicsEntry;

static struct {
	IntMap mEntries;
} gData;

static void loadBlitzPhysicsHandler(void* tData) {
	(void)tData;
	gData.mEntries = new_int_map();
}

static void updateSinglePhysicsEntry(void* tCaller, void* tData) {
	(void)tCaller;
	PhysicsEntry* e = tData;

	e->mAcceleration = vecAdd(e->mAcceleration, e->mGravity);
	e->mVelocity = vecAdd(e->mVelocity, e->mAcceleration);
	e->mAcceleration = makePosition(0, 0, 0);
	Position* pos = getBlitzEntityPositionReference(e->mEntityID);

	if (hasBlitzCollidedBottom(e->mEntityID)) {
		e->mVelocity.y = min(0, e->mVelocity.y);
	}
	if (hasBlitzCollidedTop(e->mEntityID)) {
		e->mVelocity.y = max(0, e->mVelocity.y);
	}
	//printf("phys %d %f\n", hasBlitzCollidedBottom(e->mEntityID), e->mVelocity.y);

	*pos = vecAdd(*pos, e->mVelocity);	
}

static void updateBlitzPhysicsHandler(void* tData) {
	(void)tData;
	int_map_map(&gData.mEntries, updateSinglePhysicsEntry, NULL);
}

static void unregisterEntity(int tEntityID);

static BlitzComponent BlitzPhysicsComponent = {
	.mUnregisterEntity = unregisterEntity,
};

static PhysicsEntry* getBlitzPhysicsEntry(int tEntityID) {
	if (!int_map_contains(&gData.mEntries, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have physics component.", tEntityID);
		abortSystem();
	}

	return int_map_get(&gData.mEntries, tEntityID);
}

ActorBlueprint BlitzPhysicsHandler = {
	.mLoad = loadBlitzPhysicsHandler,
	.mUpdate = updateBlitzPhysicsHandler,
};

void addBlitzPhysicsComponent(int tEntityID)
{
	PhysicsEntry* e = allocMemory(sizeof(PhysicsEntry));
	e->mEntityID = tEntityID;
	e->mVelocity = makePosition(0, 0, 0);
	e->mAcceleration = makePosition(0, 0, 0);
	e->mGravity = makePosition(0, 0, 0);
	e->mOneMinusDragOnCollision = makePosition(1, 1, 1);

	registerBlitzComponent(tEntityID, BlitzPhysicsComponent);
	int_map_push_owned(&gData.mEntries, tEntityID, e);
}

void setBlitzPhysicsGravity(int tEntityID, Acceleration tGravity)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mGravity = tGravity;
}

void addBlitzPhysicsImpulse(int tEntityID, Acceleration tImpulse)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mAcceleration = vecAdd(e->mAcceleration, tImpulse);
}

void setBlitzPhysicsDragFactorOnCollision(int tEntityID, Vector3D tDragFactor)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mOneMinusDragOnCollision = vecSub(makePosition(1, 1, 1), tDragFactor);
}

Velocity getBlitzPhysicsVelocity(int tEntityID)
{
	if (!int_map_contains(&gData.mEntries, tEntityID)) return makePosition(0, 0, 0);
	
	PhysicsEntry* e = int_map_get(&gData.mEntries, tEntityID);
	return e->mVelocity;
}

void setBlitzPhysicsVelocity(int tEntityID, Velocity tVelocity)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mVelocity = tVelocity;
}

void setBlitzPhysicsVelocityX(int tEntityID, double tX)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mVelocity.x = tX;
}

void addBlitzPhysicsVelocityX(int tEntityID, double tX)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mVelocity.x += tX;
}

double getBlitzPhysicsVelocityY(int tEntityID)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	return e->mVelocity.y;
}

void setBlitzPhysicsVelocityY(int tEntityID, double tY)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mVelocity.y = tY;
}

void addBlitzPhysicsVelocityY(int tEntityID, double tY)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mVelocity.y += tY;
}

static void unregisterEntity(int tEntityID) {
	int_map_remove(&gData.mEntries, tEntityID);
}