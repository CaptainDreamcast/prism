#include "include/datastructures.h"

#include <stdlib.h>

static int list_push_front_internal(List* tList, void* tData, int tIsOwned) {
	ListElement* e = malloc(sizeof(ListElement));
	e->mID = tList->mIDs++;
	e->mData = tData;
	e->mIsDataOwned = tIsOwned;
	e->mNext = NULL;
	e->mPrev = NULL;

	if(tList->mFirst != NULL) {
		tList->mFirst->mPrev = e;
		e->mNext = tList->mFirst;
	}
	if(tList->mLast == NULL) tList->mLast = e;

	tList->mFirst = e;

	tList->mSize++;

	return e->mID;
}

int list_push_front(List* tList, void* tData) {
	return list_push_front_internal(tList, tData, 0);
}
int list_push_front_owned(List* tList, void* tData){
	return list_push_front_internal(tList, tData, 1);
}

static void removeListElement(List* tList, ListElement* e){
	if(e == tList->mFirst) {
		tList->mFirst = e->mNext;
	}
	if(e == tList->mLast) {
		tList->mLast = e->mPrev;
	}
	if(e->mPrev != NULL) {
		e->mPrev->mNext = e->mNext;
	}
	if(e->mNext != NULL) {
		e->mNext->mPrev = e->mPrev;
	}

	if(e->mIsDataOwned) {
		free(e->mData);
	}

	free(e);
	tList->mSize--;
}

void list_remove(List* tList, int tID){
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		ListElement* next = cur->mNext;
		if(cur->mID == tID) { 
			removeListElement(tList, cur);
		}

		cur = next;
	}

}

static int trueFunction(void* tCaller, void* tData){
	(void) tCaller;
	(void) tData;
	return 1;
}

void list_empty(List* tList){
	list_remove_predicate(tList, trueFunction, NULL);
}


void list_map(List* tList, mapCB tCB, void* tCaller) {
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		tCB(tCaller, cur->mData);
		cur = cur->mNext;
	}


}

void list_remove_predicate(List* tList, predicateCB tCB, void* tCaller){
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		ListElement* next = cur->mNext;
		if(tCB(tCaller, cur->mData)) { 
			removeListElement(tList, cur);
		}

		cur = next;
	}

}


int list_size(List* tList){
	return tList->mSize;
}


List new_list() {
	List l;
	l.mSize = 0;
	l.mFirst = NULL;
	l.mIDs = 0;
	return l;
}

void delete_list(List* tList){
	list_empty(tList);
}
