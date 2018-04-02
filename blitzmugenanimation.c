#include "prism/blitzmugenanimation.h"

#include "prism/datastructures.h"
#include "prism/blitzentity.h"
#include "prism/blitzcamerahandler.h"
#include "prism/log.h"
#include "prism/system.h"

typedef struct {
	int mEntityID;

	MugenSpriteFile* mSprites;
	MugenAnimations* mAnimations;
	int mIsStatic;

	int mAnimationID;
} BlitzAnimationEntry;

static struct {
	IntMap mEntities;
} gData;

static void loadBlitzMugenAnimationHandler(void* tData) {
	(void)tData;
	gData.mEntities = new_int_map();
}

ActorBlueprint BlitzMugenAnimationHandler = {
	.mLoad = loadBlitzMugenAnimationHandler,
};

static void unregisterEntity(int tEntityID);

static BlitzComponent BlitzMugenAnimationComponent = {
	.mUnregisterEntity = unregisterEntity,
};

static BlitzAnimationEntry* getBlitzAnimationEntry(int tEntityID) {
	if (!int_map_contains(&gData.mEntities, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have animation component.", tEntityID);
		abortSystem();
	}

	return int_map_get(&gData.mEntities, tEntityID);
}

void addBlitzMugenAnimationComponentGeneral(int tEntityID, MugenSpriteFile * tSprites, MugenAnimations * tAnimations, MugenAnimation* tStartAnimation, int tIsStatic)
{
	BlitzAnimationEntry* e = allocMemory(sizeof(BlitzAnimationEntry));
	e->mEntityID = tEntityID;
	e->mSprites = tSprites;
	e->mAnimations = tAnimations;
	e->mAnimationID = addMugenAnimation(tStartAnimation, tSprites, makePosition(0, 0, 0));
	e->mIsStatic = tIsStatic;
	setMugenAnimationBasePosition(e->mAnimationID, getBlitzEntityPositionReference(tEntityID));
	setMugenAnimationScaleReference(e->mAnimationID, getBlitzEntityScaleReference(tEntityID));
	setMugenAnimationAngleReference(e->mAnimationID, getBlitzEntityRotationZReference(tEntityID));
	if (isBlitzCameraHandlerEnabled()) {
		setMugenAnimationCameraPositionReference(e->mAnimationID, getBlitzCameraHandlerPositionReference());
		setMugenAnimationCameraScaleReference(e->mAnimationID, getBlitzCameraHandlerScaleReference());
		setMugenAnimationCameraAngleReference(e->mAnimationID, getBlitzCameraHandlerRotationZReference());
	}

	registerBlitzComponent(tEntityID, BlitzMugenAnimationComponent);
	int_map_push_owned(&gData.mEntities, tEntityID, e);
}

void addBlitzMugenAnimationComponent(int tEntityID, MugenSpriteFile * tSprites, MugenAnimations * tAnimations, int tStartAnimation)
{
	addBlitzMugenAnimationComponentGeneral(tEntityID, tSprites, tAnimations, getMugenAnimation(tAnimations, tStartAnimation), 0);
}

void addBlitzMugenAnimationComponentStatic(int tEntityID, MugenSpriteFile * tSprites, int tSpriteGroup, int tSpriteItem)
{
	addBlitzMugenAnimationComponentGeneral(tEntityID, tSprites, NULL, createOneFrameMugenAnimationForSprite(tSpriteGroup, tSpriteItem), 1);
}

static void unregisterEntity(int tEntityID) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	removeMugenAnimation(e->mAnimationID);
	int_map_remove(&gData.mEntities, tEntityID);
}

int getBlitzMugenAnimationID(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return e->mAnimationID;
}

void changeBlitzMugenAnimation(int tEntityID, int tAnimationNumber)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	changeMugenAnimation(e->mAnimationID, getMugenAnimation(e->mAnimations, tAnimationNumber));
}

void changeBlitzMugenAnimationWithStartStep(int tEntityID, int tAnimationNumber, int tStep) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	changeMugenAnimationWithStartStep(e->mAnimationID, getMugenAnimation(e->mAnimations, tAnimationNumber), tStep);
}

void changeBlitzMugenAnimationIfDifferent(int tEntityID, int tAnimationNumber) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	if (getMugenAnimationAnimationNumber(e->mAnimationID) == tAnimationNumber) return;

	changeMugenAnimation(e->mAnimationID, getMugenAnimation(e->mAnimations, tAnimationNumber));
}

int getBlitzMugenAnimationAnimationNumber(int tEntityID) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationAnimationNumber(e->mAnimationID);
}

void setBlitzMugenAnimationTransparency(int tEntityID, double tTransparency) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationTransparency(e->mAnimationID, tTransparency);
}

void setBlitzMugenAnimationFaceDirection(int tEntityID, int tIsFacingRight) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationFaceDirection(e->mAnimationID, tIsFacingRight);
}