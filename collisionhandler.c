#include "include/collisionhandler.h"

#include <stdlib.h>

#include "include/datastructures.h"
#include "include/memoryhandler.h"

typedef struct {
	int mListID;
	Collider mCollider;

	CollisionCallback mCB;
	void* mCaller;

	void* mCollisionData;

	int mIsScheduledForDeletion;

} CollisionListElement;

typedef struct {
	List mCollisionElements;
} CollisionListData;

typedef struct {

	int mID1;
	int mID2;

} CollisionListPair;

static struct {
	List mCollisionLists;
	List mCollisionListPairs;	
} gData;

void setupCollisionHandler() {
	gData.mCollisionLists = new_list();
	gData.mCollisionListPairs = new_list();
}

static void destroyCollisionElement(CollisionListElement* e) {
	destroyCollider(&e->mCollider);
}

static void cleanSingleCollisionListElement(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListElement* data = tData;
	destroyCollisionElement(data);
}

static void cleanSingleCollisionList(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListData* data = tData;
	list_map(&data->mCollisionElements, cleanSingleCollisionListElement, NULL);
	list_empty(&data->mCollisionElements);
}

void shutdownCollisionHandler() {
	list_map(&gData.mCollisionLists, cleanSingleCollisionList, NULL);
	list_empty(&gData.mCollisionLists);
	list_empty(&gData.mCollisionListPairs);
}

static void checkCollisionElements(void* tCaller, void* tData) {
	CollisionListElement* e1 = tCaller;
	CollisionListElement* e2 = tData;

	
	if(checkCollisionCollider(e1->mCollider, e2->mCollider)) {
		e1->mCB(e1->mCaller, e2->mCollisionData);
		e2->mCB(e2->mCaller, e1->mCollisionData);
	}
}

static void checkAgainstOtherList(void* tCaller, void *tData) {
	List* list = tCaller;
	CollisionListElement* e = tData;

	list_map(list, checkCollisionElements, e);
}

static int checkElementRemoval(void* tCaller, void * tData) {
	(void) tCaller;
	CollisionListElement* e = tData;

	return e->mIsScheduledForDeletion;
}

static void updateSingleCollisionPair(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListPair* data = tData;

	CollisionListData* list1 = list_get(&gData.mCollisionLists, data->mID1);
	CollisionListData* list2 = list_get(&gData.mCollisionLists, data->mID1);

	list_map(&list1->mCollisionElements, checkAgainstOtherList, list2);
	
	list_remove_predicate(&list1->mCollisionElements, checkElementRemoval, NULL);	
}

void updateCollisionHandler() {
	list_map(&gData.mCollisionListPairs, updateSingleCollisionPair, NULL);
}


int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	CollisionListData* list = list_get(&gData.mCollisionLists, tListID);
	
	CollisionListElement* e = allocMemory(sizeof(CollisionListElement));
	e->mListID = tListID;

	e->mCollider = tCollider;
	setColliderBasePosition(&e->mCollider, tBasePosition);

	e->mCB = tCB;
	e->mCaller = tCaller;

	e->mCollisionData = tCollisionData;

	e->mIsScheduledForDeletion = 0;

	return list_push_front_owned(&list->mCollisionElements, e);
}


void addCollisionHandlerCheck(int tListID1, int tListID2) {
	CollisionListPair* e = allocMemory(sizeof(CollisionListPair));
	e->mID1 = tListID1;
	e->mID2 = tListID2;

	list_push_front_owned(&gData.mCollisionListPairs, e);
}

int addCollisionListToHandler() {
	CollisionListData* e = allocMemory(sizeof(CollisionListData));
	e->mCollisionElements = new_list();

	return list_push_front_owned(&gData.mCollisionLists, e);
}

