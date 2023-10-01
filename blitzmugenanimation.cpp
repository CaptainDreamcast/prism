#include "prism/blitzmugenanimation.h"

#include "prism/datastructures.h"
#include "prism/blitzentity.h"
#include "prism/blitzcamerahandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/stlutil.h"

using namespace std;

typedef struct {
	int mEntityID;

	MugenSpriteFile* mSprites;
	MugenAnimations* mAnimations;
	int mIsStatic;

	void(*mCB)(void*);
	void* mCaller;

	MugenAnimationHandlerElement* mAnimationElement;
} BlitzAnimationEntry;

static struct {
	map<int, BlitzAnimationEntry> mEntities;
} gBlitzAnimationData;

static void loadBlitzMugenAnimationHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gBlitzAnimationData.mEntities.clear();
}

static void unloadBlitzMugenAnimationHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gBlitzAnimationData.mEntities.clear();
}


ActorBlueprint getBlitzMugenAnimationHandler() {
	return makeActorBlueprint(loadBlitzMugenAnimationHandler, unloadBlitzMugenAnimationHandler);
}

static void unregisterEntity(int tEntityID);

BlitzComponent getBlitzMugenAnimationComponent() {
	return makeBlitzComponent(unregisterEntity);
}

static BlitzAnimationEntry* getBlitzAnimationEntry(int tEntityID) {
	if (!stl_map_contains(gBlitzAnimationData.mEntities, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have animation component.", tEntityID);
		recoverFromError();
	}

	return &gBlitzAnimationData.mEntities[tEntityID];
}

void addBlitzMugenAnimationComponentGeneral(int tEntityID, MugenSpriteFile * tSprites, MugenAnimations * tAnimations, MugenAnimation* tStartAnimation, int tIsStatic)
{
	BlitzAnimationEntry e;
	e.mEntityID = tEntityID;
	e.mSprites = tSprites;
	e.mAnimations = tAnimations;
	e.mAnimationElement = addMugenAnimation(tStartAnimation, tSprites, Vector3D(0, 0, 0));
	e.mIsStatic = tIsStatic;
	e.mCB = NULL;
	setMugenAnimationBasePosition(e.mAnimationElement, getBlitzEntityPositionReference(tEntityID));
	setMugenAnimationScaleReference(e.mAnimationElement, getBlitzEntityScaleReference(tEntityID));
	setMugenAnimationAngleReference(e.mAnimationElement, getBlitzEntityRotationZReference(tEntityID));
	if (isBlitzCameraHandlerEnabled()) {
		setMugenAnimationCameraPositionReference(e.mAnimationElement, getBlitzCameraHandlerPositionReference());
		setMugenAnimationCameraScaleReference(e.mAnimationElement, getBlitzCameraHandlerScaleReference());
		setMugenAnimationCameraAngleReference(e.mAnimationElement, getBlitzCameraHandlerRotationZReference());
		setMugenAnimationCameraEffectPositionReference(e.mAnimationElement, getBlitzCameraHandlerEffectPositionReference());
	}
	
	registerBlitzComponent(tEntityID, getBlitzMugenAnimationComponent());
	gBlitzAnimationData.mEntities[tEntityID] = e;
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
	removeMugenAnimation(e->mAnimationElement);
	gBlitzAnimationData.mEntities.erase(tEntityID);
}

MugenAnimationHandlerElement* getBlitzMugenAnimationElement(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return e->mAnimationElement;
}

void changeBlitzMugenAnimation(int tEntityID, int tAnimationNumber)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	changeMugenAnimation(e->mAnimationElement, getMugenAnimation(e->mAnimations, tAnimationNumber));
}

void changeBlitzMugenAnimationWithStartStep(int tEntityID, int tAnimationNumber, int tStep) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	changeMugenAnimationWithStartStep(e->mAnimationElement, getMugenAnimation(e->mAnimations, tAnimationNumber), tStep);
}

void changeBlitzMugenAnimationIfDifferent(int tEntityID, int tAnimationNumber) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	if (getMugenAnimationAnimationNumber(e->mAnimationElement) == tAnimationNumber) return;

	changeMugenAnimation(e->mAnimationElement, getMugenAnimation(e->mAnimations, tAnimationNumber));
}

void setBlitzMugenAnimationSprites(int tEntityID, MugenSpriteFile* tSprites)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationSprites(e->mAnimationElement, tSprites);
}

void setBlitzMugenAnimationAnimations(int tEntityID, MugenAnimations* tAnimations)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	e->mAnimations = tAnimations;
}

Position getBlitzMugenAnimationPosition(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationPosition(e->mAnimationElement);
}

int getBlitzMugenAnimationAnimationNumber(int tEntityID) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationAnimationNumber(e->mAnimationElement);
}

int getBlitzMugenAnimationAnimationStep(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationAnimationStep(e->mAnimationElement);
}

int getBlitzMugenAnimationAnimationStepAmount(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationAnimationStepAmount(e->mAnimationElement);
}

int getBlitzMugenAnimationAnimationStepDuration(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationAnimationStepDuration(e->mAnimationElement);
}

int getBlitzMugenAnimationRemainingAnimationTime(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationRemainingAnimationTime(e->mAnimationElement);
}

int getBlitzMugenAnimationTime(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationTime(e->mAnimationElement);
}

int getBlitzMugenAnimationDuration(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationDuration(e->mAnimationElement);
}

Vector3DI getBlitzMugenAnimationSprite(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationSprite(e->mAnimationElement);
}

int getBlitzMugenAnimationIsFacingRight(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationIsFacingRight(e->mAnimationElement);
}

int getBlitzMugenAnimationIsFacingDown(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationIsFacingDown(e->mAnimationElement);
}

int getBlitzMugenAnimationVisibility(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationVisibility(e->mAnimationElement);
}

Vector2D getBlitzMugenAnimationDrawScale(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationDrawScale(e->mAnimationElement);
}

double getBlitzMugenAnimationDrawAngle(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationDrawAngle(e->mAnimationElement);
}

double getBlitzMugenAnimationColorRed(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationColorRed(e->mAnimationElement);
}

double getBlitzMugenAnimationColorGreen(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationColorGreen(e->mAnimationElement);
}

double getBlitzMugenAnimationColorBlue(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationColorBlue(e->mAnimationElement);
}

double* getBlitzMugenAnimationColorRedReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationColorRedReference(e->mAnimationElement);
}

double* getBlitzMugenAnimationColorGreenReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationColorGreenReference(e->mAnimationElement);
}

double* getBlitzMugenAnimationColorBlueReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationColorBlueReference(e->mAnimationElement);
}

double* getBlitzMugenAnimationTransparencyReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationTransparencyReference(e->mAnimationElement);
}

double* getBlitzMugenAnimationScaleXReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationScaleXReference(e->mAnimationElement);
}

double* getBlitzMugenAnimationScaleYReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationScaleYReference(e->mAnimationElement);
}

double* getBlitzMugenAnimationBaseScaleReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationBaseScaleReference(e->mAnimationElement);
}

Position* getBlitzMugenAnimationPositionReference(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationPositionReference(e->mAnimationElement);
}

void setBlitzMugenAnimationIndependentOfCamera(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	removeMugenAnimationCameraPositionReference(e->mAnimationElement);
	removeMugenAnimationCameraScaleReference(e->mAnimationElement);
	removeMugenAnimationCameraAngleReference(e->mAnimationElement);
	removeMugenAnimationCameraEffectPositionReference(e->mAnimationElement);
}

void setBlitzMugenAnimationTransparency(int tEntityID, double tTransparency) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationTransparency(e->mAnimationElement, tTransparency);
}

void setBlitzMugenAnimationFaceDirection(int tEntityID, int tIsFacingRight) {
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationFaceDirection(e->mAnimationElement, tIsFacingRight);
}

void setBlitzMugenAnimationVerticalFaceDirection(int tEntityID, int tIsFacingDown)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationVerticalFaceDirection(e->mAnimationElement, tIsFacingDown);
}

void setBlitzMugenAnimationRectangleWidth(int tEntityID, int tWidth)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationRectangleWidth(e->mAnimationElement, tWidth);
}

void setBlitzMugenAnimationRectangleHeight(int tEntityID, int tHeight)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationRectangleHeight(e->mAnimationElement, tHeight);
}

void setBlitzMugenAnimationPositionX(int tEntityID, double tX)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	Position p = getMugenAnimationPosition(e->mAnimationElement);
	p.x = tX;
	setMugenAnimationPosition(e->mAnimationElement, p);
}

void setBlitzMugenAnimationPositionY(int tEntityID, double tY)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	Position p = getMugenAnimationPosition(e->mAnimationElement);
	p.y = tY;
	setMugenAnimationPosition(e->mAnimationElement, p);
}

void setBlitzMugenAnimationVisibility(int tEntityID, int tVisibility)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationVisibility(e->mAnimationElement, tVisibility);
}

void setBlitzMugenAnimationBlendType(int tEntityID, BlendType tBlendType)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationBlendType(e->mAnimationElement, tBlendType);
}

void setBlitzMugenAnimationConstraintRectangle(int tEntityID, const GeoRectangle2D& tConstraintRectangle)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationConstraintRectangle(e->mAnimationElement, tConstraintRectangle);
}

void setBlitzMugenAnimationColor(int tEntityID, double tR, double tG, double tB)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationColor(e->mAnimationElement, tR, tG, tB);
}

void setBlitzMugenAnimationBaseDrawScale(int tEntityID, double tScale)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationBaseDrawScale(e->mAnimationElement, tScale);
}

void setBlitzMugenAnimationDrawScale(int tEntityID, const Vector2D& tScale)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationDrawScale(e->mAnimationElement, tScale);
}

void setBlitzMugenAnimationDrawSize(int tEntityID, const Vector2D& tSize)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationDrawSize(e->mAnimationElement, tSize);
}

void setBlitzMugenAnimationAngle(int tEntityID, double tRotation)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationDrawAngle(e->mAnimationElement, tRotation);
}

static void blitzMugenAnimationFinishedCB(void* tCaller) {
	BlitzAnimationEntry* e = (BlitzAnimationEntry*)tCaller;
	if (e->mCB) {
		e->mCB(e->mCaller);
	}
	removeBlitzEntity(e->mEntityID);
}

void setBlitzMugenAnimationNoLoop(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationNoLoop(e->mAnimationElement);
	setMugenAnimationCallback(e->mAnimationElement, blitzMugenAnimationFinishedCB, e);
}

void setBlitzMugenAnimationCallback(int tEntityID, void(*tFunc)(void *), void * tCaller)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	e->mCB = tFunc;
	e->mCaller = tCaller;
}

void setBlitzMugenAnimationAnimationStepDuration(int tEntityID, int tDuration)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationAnimationStepDuration(e->mAnimationElement, tDuration);
}

void pauseBlitzMugenAnimation(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	pauseMugenAnimation(e->mAnimationElement);
}

void unpauseBlitzMugenAnimation(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	unpauseMugenAnimation(e->mAnimationElement);
}

int isStartingBlitzMugenAnimationElementWithID(int tEntityID, int tStepID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return isStartingMugenAnimationElementWithID(e->mAnimationElement, tStepID);
}

int getTimeFromBlitzMugenAnimationElement(int tEntityID, int tStep)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getTimeFromMugenAnimationElement(e->mAnimationElement, tStep);
}

int getBlitzMugenAnimationElementFromTimeOffset(int tEntityID, int tTime)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationElementFromTimeOffset(e->mAnimationElement, tTime);
}

int isBlitzMugenAnimationTimeOffsetInAnimation(int tEntityID, int tTime)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return isMugenAnimationTimeOffsetInAnimation(e->mAnimationElement, tTime);
}

int getBlitzMugenAnimationTimeWhenStepStarts(int tEntityID, int tStep)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	return getMugenAnimationTimeWhenStepStarts(e->mAnimationElement, tStep);
}

void setBlitzMugenAnimationCollisionDebug(int tEntityID, int tIsActive)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationCollisionDebug(e->mAnimationElement, tIsActive);
}

void setBlitzMugenAnimationInvisible(int tEntityID)
{
	BlitzAnimationEntry* e = getBlitzAnimationEntry(tEntityID);
	setMugenAnimationInvisible(e->mAnimationElement);
}
