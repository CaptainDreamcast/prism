#include "prism/blitzentity.h"

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/blitzcamerahandler.h"

typedef struct {
	int mID;
	Position mPosition;
	Vector3D mScale;
	double mAngle;

	Vector mComponents; // contains owned BlitzComponent copy

	int mIsMarkedForDeletion;
} BlitzEntity;

static struct {
	IntMap mEntities; // contains BlitzEntity
} gData;

static void loadBlitzMugenAnimationHandler(void* tData) {
	(void)tData;
	gData.mEntities = new_int_map();
}

static void unregisterSingleEntityComponent(void* tCaller, void* tData) {
	BlitzEntity* entity = tCaller;
	BlitzComponent* component = tData;

	component->mUnregisterEntity(entity->mID);
}

static void unloadBlitzEntity(BlitzEntity* e)
{
	vector_map(&e->mComponents, unregisterSingleEntityComponent, e);
}


static int updateSingleEntity(void* tCaller, void* tData) {
	(void)tCaller;
	BlitzEntity* e = tData;
	if (e->mIsMarkedForDeletion) {
		unloadBlitzEntity(e);
		return 1;
	}

	return 0;
}

static void updateBlitzMugenAnimationHandler(void* tData) {
	(void)tData;
	int_map_remove_predicate(&gData.mEntities, updateSingleEntity, NULL);
}

// TODO: unload
ActorBlueprint BlitzEntityHandler = {
	.mLoad = loadBlitzMugenAnimationHandler,
	.mUpdate = updateBlitzMugenAnimationHandler,
};

int addBlitzEntity(Position tPos)
{
	BlitzEntity* e = allocMemory(sizeof(BlitzEntity));
	e->mPosition = tPos;
	e->mScale = makePosition(1, 1, 1);
	e->mAngle = 0;
	e->mComponents = new_vector();
	e->mIsMarkedForDeletion = 0;
	e->mID = int_map_push_back_owned(&gData.mEntities, e);
	return e->mID;
}

static BlitzEntity* getBlitzEntity(int tID) {
	if (!int_map_contains(&gData.mEntities, tID)) {
		logErrorFormat("Unable to find entity %d", tID);
		abortSystem();
	}

	return int_map_get(&gData.mEntities, tID);
}

void removeBlitzEntity(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		logError("Unable to remove camera entity");
		abortSystem();
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mIsMarkedForDeletion = 1;
}

void registerBlitzComponent(int tID, BlitzComponent tComponent)
{
	if (tID == getBlitzCameraHandlerEntityID()) return;

	BlitzEntity* e = getBlitzEntity(tID);
	BlitzComponent* component = allocMemory(sizeof(BlitzComponent));
	*component = tComponent;

	vector_push_back_owned(&e->mComponents, component);
}

void setBlitzEntityPosition(int tID, Position tPos)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerPosition(tPos);
		return;
	}
	
	BlitzEntity* e = getBlitzEntity(tID);
	e->mPosition = tPos;
}

void setBlitzEntityPositionX(int tID, double tX)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerPositionX(tX);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mPosition.x = tX;
}

void setBlitzEntityPositionY(int tID, double tY)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerPositionY(tY);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mPosition.y = tY;
}

void setBlitzEntityScale2D(int tID, double tScale)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerScale2D(tScale);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mScale = makePosition(tScale, tScale, 1);
}

void setBlitzEntityScaleX(int tID, double tScaleX)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerScaleX(tScaleX);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mScale.x = tScaleX;
}

void setBlitzEntityScaleY(int tID, double tScaleY)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerScaleY(tScaleY);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mScale.x = tScaleY;
}

void setBlitzEntityRotationZ(int tID, double tAngle)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerRotationZ(tAngle);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	e->mAngle = tAngle;
}

Position getBlitzEntityPosition(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerPosition();
	}

	BlitzEntity* e = getBlitzEntity(tID);
	return e->mPosition;
}

Position * getBlitzEntityPositionReference(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerPositionReference();
	}

	BlitzEntity* e = getBlitzEntity(tID);
	return &e->mPosition;
}

Vector3D * getBlitzEntityScaleReference(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerScaleReference();
	}

	BlitzEntity* e = getBlitzEntity(tID);
	return &e->mScale;
}

double * getBlitzEntityRotationZReference(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerRotationZReference();
	}
	BlitzEntity* e = getBlitzEntity(tID);
	return &e->mAngle;
}
