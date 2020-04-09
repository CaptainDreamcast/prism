#include "prism/collisionhandler.h"

#include <stdlib.h>
#include <stdio.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/drawing.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/screeneffect.h"
#include "prism/geometry.h"
#include <prism/stlutil.h>

using namespace std;

typedef struct {

	CollisionListData* mList1;
	CollisionListData* mList2;
} CollisionListPair;

typedef struct {
	int mIsActive;
	TextureData mWhiteCircleTexture;

	Position* mScreenPositionReference;
} CollisionHandlerDebugData;

static struct {
	map<int, CollisionListData> mCollisionLists;
	map<int, CollisionListPair> mCollisionListPairs;

	CollisionHandlerDebugData mDebug;

	int mIsOwningColliders;
} gCollisionHandler;

void setupCollisionHandler() {
	gCollisionHandler.mCollisionLists.clear();
	gCollisionHandler.mCollisionListPairs.clear();
	gCollisionHandler.mIsOwningColliders = 0;
}

static void destroyCollisionElement(CollisionListElement* e) {
	if (gCollisionHandler.mIsOwningColliders || e->mIsColliderOwned) {
		destroyCollider(&e->mCollider);
	}
}

static void cleanSingleCollisionListElement(CollisionListElement& tData) {
	destroyCollisionElement(&tData);
}

static void cleanSingleCollisionList(CollisionListData& tData) {
	CollisionListData* data = &tData;
	stl_int_map_map(data->mCollisionElements, cleanSingleCollisionListElement);
	data->mCollisionElements.clear();
}

void shutdownCollisionHandler() {
	stl_int_map_map(gCollisionHandler.mCollisionLists, cleanSingleCollisionList);
	gCollisionHandler.mCollisionLists.clear();
	gCollisionHandler.mCollisionListPairs.clear();
}

static void checkCollisionElements(CollisionListElement* e1, CollisionListElement& tData) {
	CollisionListElement* e2 = &tData;

	if (e1->mIsScheduledForDeletion || e2->mIsScheduledForDeletion) return;

	if(checkCollisionCollider(e1->mCollider, e2->mCollider)) {
		if(e1->mCB) e1->mCB(e1->mCaller, e2->mCollisionData, e2->mListID);
		if(e2->mCB) e2->mCB(e2->mCaller, e1->mCollisionData, e1->mListID);
	}
}

static void checkAgainstOtherList(CollisionListData* tList, CollisionListElement& tData) {
	CollisionListElement* e = &tData;

	if (e->mIsScheduledForDeletion) return;

	stl_int_map_map(tList->mCollisionElements, checkCollisionElements, e);
}

static int checkElementRemoval(CollisionListElement& tData) {
	CollisionListElement* e = &tData;
	
	if (e->mIsScheduledForDeletion) {
		destroyCollisionElement(e);
	}

	return e->mIsScheduledForDeletion;
}

static void updateSingleCollisionPair(CollisionListPair& tData) {
	CollisionListPair* data = &tData;

	CollisionListData* list1 = data->mList1;
	CollisionListData* list2 = data->mList2;

	stl_int_map_remove_predicate(list1->mCollisionElements, checkElementRemoval);
	stl_int_map_remove_predicate(list2->mCollisionElements, checkElementRemoval);

	stl_int_map_map(list1->mCollisionElements, checkAgainstOtherList, list2);
	
	stl_int_map_remove_predicate(list1->mCollisionElements, checkElementRemoval);	
	stl_int_map_remove_predicate(list2->mCollisionElements, checkElementRemoval);

}

void updateCollisionHandler() {
	stl_int_map_map(gCollisionHandler.mCollisionListPairs, updateSingleCollisionPair);
}

static void setColliderOwned(CollisionListElement* e) {
	e->mIsColliderOwned = 1;
}

CollisionListElement* addCollisionRectangleToCollisionHandler(CollisionListData* tList, Position* tBasePosition, CollisionRect tRect, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	Collider collider = makeColliderFromRect(tRect);
	auto element = addColliderToCollisionHandler(tList, tBasePosition, collider, tCB, tCaller, tCollisionData);
	setColliderOwned(element);
	return element;
}

void changeCollisionRectangleInCollisionHandler(CollisionListElement* e, CollisionRect tRect)
{
	e->mCollider.mImpl.mRect = tRect;
}

CollisionListElement* addCollisionCircleToCollisionHandler(CollisionListData* tList, Position* tBasePosition, CollisionCirc tCirc, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	Collider collider = makeColliderFromCirc(tCirc);
	auto element = addColliderToCollisionHandler(tList, tBasePosition, collider, tCB, tCaller, tCollisionData);
	setColliderOwned(element);
	return element;
}

CollisionListElement* addColliderToCollisionHandler(CollisionListData* tList, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData) {	
	CollisionListElement e;
	e.mCollider = tCollider;
	e.mIsColliderOwned = 0;

	e.mCB = tCB;
	e.mCaller = tCaller;

	e.mCollisionData = tCollisionData;

	e.mIsScheduledForDeletion = 0;

	int id = stl_int_map_push_back(tList->mCollisionElements, e);
	auto element = &tList->mCollisionElements[id];
	element->mID = id;
	element->mListID = tList->mID;
	setColliderBasePosition(&element->mCollider, tBasePosition);
	return element;
}


void addCollisionHandlerCheck(CollisionListData* tList1, CollisionListData* tList2) {
	CollisionListPair e;
	e.mList1 = tList1;
	e.mList2 = tList2;

	stl_int_map_push_back(gCollisionHandler.mCollisionListPairs, e);
}

CollisionListData* addCollisionListToHandler() {
	CollisionListData e;
	e.mCollisionElements.clear();
	int id = stl_int_map_push_back(gCollisionHandler.mCollisionLists, e);
	auto list = &gCollisionHandler.mCollisionLists[id];
	list->mID = id;
	return list;
}

void removeFromCollisionHandler(CollisionListElement* e) {
	e->mIsScheduledForDeletion = 1;
}

void updateColliderForCollisionHandler(CollisionListElement* e, Collider tCollider) {
	if (e->mIsColliderOwned) {
		destroyCollider(&e->mCollider);
	}

	e->mCollider = tCollider;
}

void setCollisionHandlerOwningColliders() {
	gCollisionHandler.mIsOwningColliders = 1;
}

void resolveHandledCollisionMovableStatic(CollisionListElement* e1, CollisionListElement* e2, Position* tPos1, Velocity tVel1)
{
	resolveCollisionColliderColliderMovableStatic(tPos1, tVel1, e1->mCollider, e2->mCollider);
}

int isHandledCollisionAboveOtherCollision(CollisionListElement* e1, CollisionListElement* e2)
{
	return getColliderDown(e1->mCollider) <= getColliderUp(e2->mCollider);
}

int isHandledCollisionBelowOtherCollision(CollisionListElement* e1, CollisionListElement* e2)
{
	return getColliderUp(e1->mCollider) >= getColliderDown(e2->mCollider);
}

int isHandledCollisionLeftOfOtherCollision(CollisionListElement* e1, CollisionListElement* e2)
{
	return getColliderRight(e1->mCollider) <= getColliderLeft(e2->mCollider);
}

int isHandledCollisionRightOfOtherCollision(CollisionListElement* e1, CollisionListElement* e2)
{
	return getColliderLeft(e1->mCollider) >= getColliderRight(e2->mCollider);
}

int isHandledCollisionValid(CollisionListElement* e)
{
	if (!stl_map_contains(gCollisionHandler.mCollisionLists, e->mListID)) {
		return 0;
	}
	CollisionListData* list = &gCollisionHandler.mCollisionLists[e->mListID];

	if (!stl_map_contains(list->mCollisionElements, e->mID)) {
		return 0;
	}
	return !e->mIsScheduledForDeletion;
}

int isHandledCollisionScheduledForDeletion(CollisionListElement* e)
{
	return e->mIsScheduledForDeletion;
}

#define DEBUG_Z 99

static void drawCollisionRect(CollisionRect tRect, Position tBasePosition, Position tScreenPositionOffset, Vector3D tColor, double tAlpha){
	double dx = tRect.mBottomRight.x -  tRect.mTopLeft.x;
	double dy = tRect.mBottomRight.y -  tRect.mTopLeft.y;
	
	if(dx < 0 || dy < 0) return;

	Position position = vecAdd(tRect.mTopLeft, tBasePosition);
	position = vecSub(position, tScreenPositionOffset);
	position.z = DEBUG_Z;
	
	Vector3D scale = makePosition(dx / 16.0, dy / 16.0, 1);
	scaleDrawing3D(scale, position);
	
	setDrawingBaseColorAdvanced(tColor.x, tColor.y, tColor.z);
	setDrawingTransparency(tAlpha);

	TextureData whiteTexture = getEmptyWhiteTexture();
	drawSprite(whiteTexture, position, makeRectangleFromTexture(whiteTexture));
	setDrawingParametersToIdentity();
}

static void drawCollisionCirc(CollisionCirc tCirc, Position tBasePosition, Position tScreenPositionOffset, Vector3D tColor, double tAlpha) {
	double r = tCirc.mRadius;
	double d = r * 2;

	if (r < 0) return;

	Position position = vecAdd(tCirc.mCenter, tBasePosition);
	position = vecAdd(position, vecScale(makePosition(r, r, 0), -1));
	position = vecSub(position, tScreenPositionOffset);
	position.z = DEBUG_Z;

	Vector3D scale = makePosition(d / 16.0, d / 16.0, 1);
	scaleDrawing3D(scale, position);
	
	setDrawingBaseColorAdvanced(tColor.x, tColor.y, tColor.z);
	setDrawingTransparency(tAlpha);

	drawSprite(gCollisionHandler.mDebug.mWhiteCircleTexture, position, makeRectangleFromTexture(gCollisionHandler.mDebug.mWhiteCircleTexture));
	setDrawingParametersToIdentity();
}

void drawColliderSolid(Collider tCollider, Position tOffset, Position tScreenPositionOffset, Vector3D tColor, double tAlpha) {
	Position basePosition;
	if (tCollider.mBasePosition) basePosition = *tCollider.mBasePosition;
	else basePosition = makePosition(0, 0, 0);

	basePosition = vecAdd(basePosition, tOffset);

	if (tCollider.mType == COLLISION_RECT) {
		CollisionRect* rect = (CollisionRect*)tCollider;
		drawCollisionRect(*rect, basePosition, tScreenPositionOffset, tColor, tAlpha);
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		CollisionCirc* circ = (CollisionCirc*)tCollider;
		drawCollisionCirc(*circ, basePosition, tScreenPositionOffset, tColor, tAlpha);
	}
	else {
		logError("Unable to draw collision type");
		logErrorInteger(tCollider.mType);
		recoverFromError();
	}
}

static void drawCollisionElement(CollisionListElement& tData) {
	CollisionListElement* e = &tData;
	
	Collider col = e->mCollider;

	Position screenOffset;
	if (gCollisionHandler.mDebug.mScreenPositionReference) screenOffset = *gCollisionHandler.mDebug.mScreenPositionReference;
	else screenOffset = makePosition(0, 0, 0);

	drawColliderSolid(col, makePosition(0, 0, 0), screenOffset, makePosition(1, 1, 1), 1);
}

static void drawCollisionList(CollisionListData& tData) {
	CollisionListData* list = &tData;
	
	stl_int_map_map(list->mCollisionElements, drawCollisionElement);
}

void setCollisionHandlerDebuggingScreenPositionReference(Position* tPosition) {
	gCollisionHandler.mDebug.mScreenPositionReference = tPosition;
}

void drawHandledCollisions() {
	if(!gCollisionHandler.mDebug.mIsActive) return;

	stl_int_map_map(gCollisionHandler.mCollisionLists, drawCollisionList);
}

void activateCollisionHandlerDebugMode() {
	gCollisionHandler.mDebug.mIsActive = 1;
	gCollisionHandler.mDebug.mWhiteCircleTexture = createWhiteCircleTexture();
	gCollisionHandler.mDebug.mScreenPositionReference = NULL;	
}
