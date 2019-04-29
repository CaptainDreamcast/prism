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
	int mListID;
	
	int mIsColliderOwned;
	Collider mCollider;

	CollisionCallback mCB;
	void* mCaller;

	void* mCollisionData;

	int mIsScheduledForDeletion;

} CollisionListElement;

typedef struct {
	map<int, CollisionListElement> mCollisionElements;
} CollisionListData;

typedef struct {

	int mID1;
	int mID2;

} CollisionListPair;

typedef struct {
	int mIsActive;
	
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
		if(e1->mCB) e1->mCB(e1->mCaller, e2->mCollisionData);
		if(e2->mCB) e2->mCB(e2->mCaller, e1->mCollisionData);
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

	CollisionListData* list1 = &gCollisionHandler.mCollisionLists[data->mID1];
	CollisionListData* list2 = &gCollisionHandler.mCollisionLists[data->mID2];

	stl_int_map_remove_predicate(list1->mCollisionElements, checkElementRemoval);
	stl_int_map_remove_predicate(list2->mCollisionElements, checkElementRemoval);

	stl_int_map_map(list1->mCollisionElements, checkAgainstOtherList, list2);
	
	stl_int_map_remove_predicate(list1->mCollisionElements, checkElementRemoval);	
	stl_int_map_remove_predicate(list2->mCollisionElements, checkElementRemoval);

}

void updateCollisionHandler() {
	stl_int_map_map(gCollisionHandler.mCollisionListPairs, updateSingleCollisionPair);
}

static void setColliderOwned(int tListID, int tID) {
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];
	CollisionListElement* e = &list->mCollisionElements[tID];

	e->mIsColliderOwned = 1;
}

int addCollisionRectangleToCollisionHandler(int tListID, Position* tBasePosition, CollisionRect tRect, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	Collider collider = makeColliderFromRect(tRect);
	int id = addColliderToCollisionHandler(tListID, tBasePosition, collider, tCB, tCaller, tCollisionData);
	setColliderOwned(tListID, id);
	return id;
}

void changeCollisionRectangleInCollisionHandler(int tListID, int tElementID, CollisionRect tRect)
{
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];
	auto e = &list->mCollisionElements[tElementID];
	e->mCollider.mImpl.mRect = tRect;
}

int addCollisionCircleToCollisionHandler(int tListID, Position* tBasePosition, CollisionCirc tCirc, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	Collider collider = makeColliderFromCirc(tCirc);
	int id = addColliderToCollisionHandler(tListID, tBasePosition, collider, tCB, tCaller, tCollisionData);
	setColliderOwned(tListID, id);
	return id;
}

int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];
	
	CollisionListElement e;
	e.mListID = tListID;

	e.mCollider = tCollider;
	e.mIsColliderOwned = 0;

	e.mCB = tCB;
	e.mCaller = tCaller;

	e.mCollisionData = tCollisionData;

	e.mIsScheduledForDeletion = 0;

	int id = stl_int_map_push_back(list->mCollisionElements, e);
	setColliderBasePosition(&list->mCollisionElements[id].mCollider, tBasePosition);
	return id;
}


void addCollisionHandlerCheck(int tListID1, int tListID2) {
	CollisionListPair e;
	e.mID1 = tListID1;
	e.mID2 = tListID2;

	stl_int_map_push_back(gCollisionHandler.mCollisionListPairs, e);
}

int addCollisionListToHandler() {
	CollisionListData e;
	e.mCollisionElements.clear();

	return stl_int_map_push_back(gCollisionHandler.mCollisionLists, e);
}

void removeFromCollisionHandler(int tListID, int tElementID) {
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];
	CollisionListElement* e = &list->mCollisionElements[tElementID];
	e->mIsScheduledForDeletion = 1;
}

void updateColliderForCollisionHandler(int tListID, int tElementID, Collider tCollider) {
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];
	CollisionListElement* e = &list->mCollisionElements[tElementID];

	if (e->mIsColliderOwned) {
		destroyCollider(&e->mCollider);
	}

	e->mCollider = tCollider;
}

void setCollisionHandlerOwningColliders() {
	gCollisionHandler.mIsOwningColliders = 1;
}

static CollisionListElement* getCollisionListElement(int tListID, int tElementID) {
	if (!stl_map_contains(gCollisionHandler.mCollisionLists, tListID)) {
		logErrorFormat("Collision handler does not contain list with id %d.", tListID);
		recoverFromError();
	}
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];

	if (!stl_map_contains(list->mCollisionElements, tElementID)) {
		logErrorFormat("Collision handler list %d does not contain element with id %d.", tListID, tElementID);
		recoverFromError();
	}
	return &list->mCollisionElements[tElementID];
}

void resolveHandledCollisionMovableStatic(int tListID1, int tElementID1, int tListID2, int tElementID2, Position* tPos1, Velocity tVel1)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);	
	resolveCollisionColliderColliderMovableStatic(tPos1, tVel1, e1->mCollider, e2->mCollider);
}

int isHandledCollisionAboveOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderDown(e1->mCollider) <= getColliderUp(e2->mCollider);
}

int isHandledCollisionBelowOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderUp(e1->mCollider) >= getColliderDown(e2->mCollider);
}

int isHandledCollisionLeftOfOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderRight(e1->mCollider) <= getColliderLeft(e2->mCollider);
}

int isHandledCollisionRightOfOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderLeft(e1->mCollider) >= getColliderRight(e2->mCollider);
}

int isHandledCollisionValid(int tListID, int tElementID)
{
	if (!stl_map_contains(gCollisionHandler.mCollisionLists, tListID)) {
		return 0;
	}
	CollisionListData* list = &gCollisionHandler.mCollisionLists[tListID];

	if (!stl_map_contains(list->mCollisionElements, tElementID)) {
		return 0;
	}

	CollisionListElement* e = &list->mCollisionElements[tElementID];
	return !e->mIsScheduledForDeletion;
}

int isHandledCollisionScheduledForDeletion(int tListID, int tElementID)
{
	CollisionListElement* e = getCollisionListElement(tListID, tElementID);
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

	TextureData whiteTexture = getEmptyWhiteTexture();
	drawSprite(whiteTexture, position, makeRectangleFromTexture(whiteTexture)); // TODO: circle texture
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
	gCollisionHandler.mDebug.mScreenPositionReference = NULL;	
}
