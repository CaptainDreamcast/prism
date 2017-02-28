#include "include/datastructures.h"

#include "include/memoryhandler.h"
#include "include/log.h"
#include "include/system.h"

#include <stdlib.h>

static ListElement* newListElement(List* tList, void* tData, int tIsOwned) {
	ListElement* e = allocMemory(sizeof(ListElement));
	e->mID = tList->mIDs++;
	e->mData = tData;
	e->mIsDataOwned = tIsOwned;
	e->mNext = NULL;
	e->mPrev = NULL;
	return e;
}

static int list_push_front_internal(List* tList, void* tData, int tIsOwned) {
	ListElement* e = newListElement(tList, tData, tIsOwned);

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

static int list_push_back_internal(List* tList, void* tData, int tIsOwned) {
	ListElement* e = newListElement(tList, tData,  tIsOwned);

	if(tList->mLast != NULL) {
		tList->mLast->mNext = e;
		e->mPrev = tList->mLast;
	}
	if(tList->mFirst == NULL) tList->mFirst = e;

	tList->mLast = e;
	tList->mSize++;

	return e->mID;
}

int list_push_back(List* tList, void* tData) {
	return list_push_back_internal(tList, tData, 0);
}
int list_push_back_owned(List* tList, void* tData) {
	return list_push_back_internal(tList, tData, 1);
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
		freeMemory(e->mData);
	}

	freeMemory(e);
	tList->mSize--;
}

void* list_get(List* tList, int tID) {
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		if(cur->mID == tID) { 
			return cur->mData;
		}

		cur = cur->mNext;
	}

	return NULL;
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
	l.mLast = NULL;
	l.mIDs = 0;
	return l;
}

void delete_list(List* tList){
	list_empty(tList);
}

ListIterator list_iterator_begin(List* tList) {
	return tList->mFirst;
}

void* list_iterator_get(ListIterator tIterator) {
	return tIterator->mData;
}

void list_iterator_increase(ListIterator* tIterator) {
	if((*tIterator)->mNext == NULL) {
		logError("Trying to increase end iterator.");
		abortSystem();
	}
	*tIterator = (*tIterator)->mNext;
}

int list_has_next(ListIterator tIterator)  {
	return tIterator->mNext != NULL;
}



Vector new_vector() {
	Vector ret;
	ret.mSize = 0;
	ret.mAlloc = 2;
	ret.mData = allocMemory(sizeof(VectorElement) * ret.mAlloc);
	return ret;
}

void delete_vector(Vector* tVector) {
	vector_empty(tVector);

	freeMemory(tVector->mData);
	tVector->mSize = 0;
	tVector->mAlloc = 0;
}

void vector_empty(Vector* tVector) {
	int i;
	for(i = 0; i < tVector->mSize; i++) {
		VectorElement* e = &tVector->mData[i];
		if(e->mIsOwned) freeMemory(e->mData);
	}

	tVector->mSize = 0;		
	tVector->mAlloc = 2;
	tVector->mData = reallocMemory(tVector->mData, sizeof(VectorElement)*tVector->mAlloc);
}

static void vector_push_back_internal(Vector* tVector, void* tData, int tIsOwned) {
	if(tVector->mSize == tVector->mAlloc) {
		tVector->mAlloc *= 2;
		tVector->mData = reallocMemory(tVector->mData, sizeof(VectorElement)*tVector->mAlloc);
	}

	VectorElement* e = &tVector->mData[tVector->mSize];
	e->mIsOwned = tIsOwned;
	e->mData = tData;
	tVector->mSize++;
}

void vector_push_back(Vector* tVector, void* tData) {
	vector_push_back_internal(tVector, tData, 0);
}

void vector_push_back_owned(Vector* tVector, void* tData) {
	vector_push_back_internal(tVector, tData, 1);
}

void* vector_get(Vector* tVector, int tIndex) {
	return tVector->mData[tIndex].mData;
}

int vector_size(Vector* tVector) {
	return tVector->mSize;
}

void vector_map(Vector* tVector, mapCB tCB, void* tCaller) {
	int i;
	for(i = 0; i < tVector->mSize; i++) {
		tCB(tCaller, tVector->mData[i].mData);
	}
}
