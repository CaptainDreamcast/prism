#include "prism/blitzphysics.h"

#include "prism/datastructures.h"
#include "prism/physics.h"
#include "prism/memoryhandler.h"
#include "prism/blitzentity.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/blitzcollision.h"
#include "prism/math.h"
#include "prism/stlutil.h"

#include <algorithm>

using namespace std;

typedef struct {
	int mEntityID;

	Velocity mVelocity;
	Acceleration mAcceleration;
	Acceleration mGravity;
	Vector3D mOneMinusDragOnCollision;
} PhysicsEntry;

static struct {
	map<int, PhysicsEntry> mEntries;
} gBlitzPhysicsData;

static void loadBlitzPhysicsHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gBlitzPhysicsData.mEntries.clear();
}

static void updateSinglePhysicsEntry(void* /*tCaller*/, PhysicsEntry& tData) {
	PhysicsEntry* e = &tData;

	e->mAcceleration = vecAdd(e->mAcceleration, e->mGravity);
	e->mVelocity = vecAdd(e->mVelocity, e->mAcceleration);
	e->mAcceleration = makePosition(0, 0, 0);
	Position* pos = getBlitzEntityPositionReference(e->mEntityID);

	if (hasBlitzCollidedBottom(e->mEntityID)) {
		e->mVelocity.y = min(0.0, e->mVelocity.y);
	}
	if (hasBlitzCollidedTop(e->mEntityID)) {
		e->mVelocity.y = max(0.0, e->mVelocity.y);
	}

	*pos = vecAdd(*pos, e->mVelocity);	
}

static void updateBlitzPhysicsHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gBlitzPhysicsData.mEntries, updateSinglePhysicsEntry);
}

static void unregisterEntity(int tEntityID);

static BlitzComponent getBlitzPhysicsComponent() {
	return makeBlitzComponent(unregisterEntity);
}

static PhysicsEntry* getBlitzPhysicsEntry(int tEntityID) {
	if (!stl_map_contains(gBlitzPhysicsData.mEntries, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have physics component.", tEntityID);
		recoverFromError();
	}

	return &gBlitzPhysicsData.mEntries[tEntityID];
}

ActorBlueprint getBlitzPhysicsHandler() {
	return makeActorBlueprint(loadBlitzPhysicsHandler, NULL, updateBlitzPhysicsHandler);
}

void addBlitzPhysicsComponent(int tEntityID)
{
	PhysicsEntry e;
	e.mEntityID = tEntityID;
	e.mVelocity = makePosition(0, 0, 0);
	e.mAcceleration = makePosition(0, 0, 0);
	e.mGravity = makePosition(0, 0, 0);
	e.mOneMinusDragOnCollision = makePosition(1, 1, 1);

	registerBlitzComponent(tEntityID, getBlitzPhysicsComponent());
	gBlitzPhysicsData.mEntries[tEntityID] = e;
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
	if (!stl_map_contains(gBlitzPhysicsData.mEntries, tEntityID)) return makePosition(0, 0, 0);
	
	PhysicsEntry* e = &gBlitzPhysicsData.mEntries[tEntityID];
	return e->mVelocity;
}

Velocity* getBlitzPhysicsVelocityReference(int tEntityID)
{
	if (!stl_map_contains(gBlitzPhysicsData.mEntries, tEntityID)) {
		logWarningFormat("Entity %d has no physics component. Returning NULL.", tEntityID);
		return NULL;
	}
	
	PhysicsEntry* e = &gBlitzPhysicsData.mEntries[tEntityID];
	return &e->mVelocity;
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

void addBlitzPhysicsVelocity(int tEntityID, Velocity tVelocity)
{
	PhysicsEntry* e = getBlitzPhysicsEntry(tEntityID);
	e->mVelocity += tVelocity;
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
	gBlitzPhysicsData.mEntries.erase(tEntityID);
}
