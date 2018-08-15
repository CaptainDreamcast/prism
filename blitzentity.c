#include "prism/blitzentity.h"

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/blitzcamerahandler.h"

typedef struct BlitzEntity_t{
	int mID;
	Position mPosition;
	Vector3D mScale;
	double mAngle;

	int mHasParent;

	struct BlitzEntity_t* mParent;
	Position mPreviousParentPosition;
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

static void updateEntityParentReferencePosition(BlitzEntity* e) {
	if (!e->mHasParent) return;

	Position parentPos = e->mParent->mPosition;
	Position delta = vecSub(parentPos, e->mPreviousParentPosition);
	e->mPosition = vecAdd(e->mPosition, delta);

	e->mPreviousParentPosition = parentPos;
}

static int updateSingleEntity(void* tCaller, void* tData) {
	(void)tCaller;
	BlitzEntity* e = tData;

	if (e->mIsMarkedForDeletion) {
		unloadBlitzEntity(e);
		return 1;
	}

	updateEntityParentReferencePosition(e);

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
	e->mHasParent = 0;
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
	if (e->mHasParent) e->mPosition = vecAdd(tPos, e->mParent->mPosition);
	else e->mPosition = tPos;
}

void setBlitzEntityPositionX(int tID, double tX)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerPositionX(tX);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	if (e->mHasParent) e->mPosition.x = tX + e->mParent->mPosition.x;
	else e->mPosition.x = tX;
}

void setBlitzEntityPositionY(int tID, double tY)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		setBlitzCameraHandlerPositionY(tY);
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	if (e->mHasParent) e->mPosition.y = tY + e->mParent->mPosition.y;
	else e->mPosition.y = tY;
}

void addBlitzEntityPositionX(int tID, double tX) {
	setBlitzEntityPositionX(tID, getBlitzEntityPositionX(tID) + tX);
}
void addBlitzEntityPositionY(int tID, double tY) {
	setBlitzEntityPositionY(tID, getBlitzEntityPositionY(tID) + tY);
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

void addBlitzEntityRotationZ(int tID, double tAngle) {
	setBlitzEntityRotationZ(tID, getBlitzEntityRotationZ(tID) + tAngle);
}

void setBlitzEntityParent(int tID, int tParentID)
{
	if (tID == getBlitzCameraHandlerEntityID() || tParentID == getBlitzCameraHandlerEntityID()) {
		logWarning("Trying to use camera in parenting system. Unimplemented. Ignoring.");
		return;
	}

	BlitzEntity* e = getBlitzEntity(tID);
	BlitzEntity* parent = getBlitzEntity(tParentID);

	e->mParent = parent;
	e->mPreviousParentPosition = parent->mPosition;
	e->mHasParent = 1;

}

Position getBlitzEntityPosition(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerPosition();
	}

	BlitzEntity* e = getBlitzEntity(tID);
	return e->mPosition;
}

double getBlitzEntityPositionX(int tID)
{
	return getBlitzEntityPosition(tID).x;
}

double getBlitzEntityPositionY(int tID)
{
	return getBlitzEntityPosition(tID).y;
}

Vector3D getBlitzEntityScale(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerScale();
	}

	BlitzEntity* e = getBlitzEntity(tID);
	return e->mScale;
}

double getBlitzEntityRotationZ(int tID)
{
	if (tID == getBlitzCameraHandlerEntityID()) {
		return getBlitzCameraHandlerRotationZ();
	}
	BlitzEntity* e = getBlitzEntity(tID);
	return e->mAngle;
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
