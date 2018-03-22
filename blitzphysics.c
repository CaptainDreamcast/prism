#include "prism/blitzphysics.h"

#include "prism/datastructures.h"
#include "prism/physics.h"
#include "prism/memoryhandler.h"
#include "prism/blitzentity.h"

typedef struct {
	int mEntityID;

} PhysicsEntry;

static struct {
	IntMap mEntries;
} gData;

static void loadBlitzPhysicsHandler(void* tData) {
	(void)tData;
	gData.mEntries = new_int_map();
}

static void updateBlitzPhysicsHandler(void* tData) {
	(void)tData;
}

static void unregisterEntity(int tEntityID);

static BlitzComponent BlitzPhysicsComponent = {
	.mUnregisterEntity = unregisterEntity,
};

ActorBlueprint BlitzPhysicsHandler = {
	.mLoad = loadBlitzPhysicsHandler,
	.mUpdate = updateBlitzPhysicsHandler,
};

void addBlitzPhysicsComponent(int tEntityID)
{
	PhysicsEntry* e = allocMemory(sizeof(PhysicsEntry));
	e->mEntityID = tEntityID;

	registerBlitzComponent(tEntityID, BlitzPhysicsComponent);
	int_map_push_owned(&gData.mEntries, tEntityID, e);
}

static void unregisterEntity(int tEntityID) {
	int_map_remove(&gData.mEntries, tEntityID);
}