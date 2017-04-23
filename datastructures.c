#include "include/datastructures.h"

#include <stdlib.h>
#include <string.h>

#include "include/memoryhandler.h"
#include "include/log.h"
#include "include/system.h"

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

void list_iterator_remove(List* tList, ListIterator tIterator) {
	removeListElement(tList, tIterator);
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

void* vector_push_back_new_buffer(Vector* tVector, int tSize) {
	char* b = allocMemory(tSize);
	vector_push_back_owned(tVector, b);
	return b;
}

void* vector_get(Vector* tVector, int tIndex) {
	return tVector->mData[tIndex].mData;
}

int vector_size(Vector* tVector) {
	return tVector->mSize;
}

void vector_remove(Vector* tVector, int ind) {
	VectorElement* e = &tVector->mData[ind];
	if (e->mIsOwned) {
		freeMemory(e->mData);
	}

	int i;
	for (i = ind; i < tVector->mSize - 1; i++) {
		tVector->mData[i] = tVector->mData[i + 1];
	}

	tVector->mSize--;
}

static sortCB gSortFunc;
static void* gSortCaller;

static int vector_sort_cmp(const void* tData1, const void* tData2) {
	const VectorElement* e1 = tData1;
	const VectorElement* e2 = tData2;

	return gSortFunc(gSortCaller, e1->mData, e2->mData);
}

void vector_sort(Vector* tVector, sortCB tCB, void* tCaller) {
	gSortCaller = tCaller;
	gSortFunc = tCB;
	qsort(tVector->mData, tVector->mSize, sizeof(VectorElement), vector_sort_cmp);
}

void vector_map(Vector* tVector, mapCB tCB, void* tCaller) {
	int i;
	for(i = 0; i < tVector->mSize; i++) {
		tCB(tCaller, tVector->mData[i].mData);
	}
}


void* vector_get_back(Vector* tVector) {
	return tVector->mData[tVector->mSize-1].mData;
}

void vector_pop_back(Vector* tVector) {
	vector_remove(tVector, tVector->mSize-1);
}




#define MAP_MODULO 31

typedef struct {
	char mKey[100];
	void* mData;
	int mIsOwned;
} StringMapBucketListEntry;

typedef struct {
	List mEntries;
} StringMapBucket;

StringMap new_string_map() {
	StringMap ret;
	ret.mBuckets = new_vector();

	int i;
	for (i = 0; i < MAP_MODULO; i++) {
		StringMapBucket* newBucket = allocMemory(sizeof(StringMapBucket));
		newBucket->mEntries = new_list();
		vector_push_back_owned(&ret.mBuckets, newBucket);
	}

	ret.mSize = 0;
	return ret;
}

static void deleteStringMapBucket(void* tCaller, void* tData) {
	(void) tCaller;
	StringMapBucket* e = tData;
	delete_list(&e->mEntries);
}

void delete_string_map(StringMap* tMap) {
	string_map_empty(tMap);
	delete_vector(&tMap->mBuckets);
}

void string_map_empty(StringMap* tMap) {
	vector_map(&tMap->mBuckets, deleteStringMapBucket, NULL);
	vector_empty(&tMap->mBuckets);
}

static int getBucketIDFromString(char* tKey) {
	int l = strlen(tKey);
	int i;
	int base = 1;
	int offset = 0;
	for (i = 0; i < l; i++) {
		offset = (offset + (tKey[i] * base)) % MAP_MODULO;
		base = (base * 10) % MAP_MODULO;
	}
	return offset;
}

static void string_map_push_internal(StringMap* tMap, char* tKey, void* tData, int tIsOwned) {
	int offset = getBucketIDFromString(tKey);
	StringMapBucket* bucket = vector_get(&tMap->mBuckets, offset);

	StringMapBucketListEntry* newEntry = allocMemory(sizeof(StringMapBucketListEntry));
	strcpy(newEntry->mKey, tKey);
	newEntry->mData = tData;
	newEntry->mIsOwned = tIsOwned;

	list_push_back(&bucket->mEntries, newEntry);
	tMap->mSize++;
}

void string_map_push_owned(StringMap* tMap, char* tKey, void* tData) {
	string_map_push_internal(tMap, tKey, tData, 1);
}

void string_map_push(StringMap* tMap, char* tKey, void* tData) {
	string_map_push_internal(tMap, tKey, tData, 0);
}

static void string_map_remove_element(StringMapBucketListEntry* e) {
	if (e->mIsOwned) {
		freeMemory(e);
	}
}

void string_map_remove(StringMap* tMap, char* tKey) {
	int offset = getBucketIDFromString(tKey);
	StringMapBucket* bucket = vector_get(&tMap->mBuckets, offset);

	if (!list_size(&bucket->mEntries)) {
		logError("Unable to find key in map.");
		logErrorString(tKey);
		abortSystem();
	}
	ListIterator it = list_iterator_begin(&bucket->mEntries);
	while (1) {
		
		StringMapBucketListEntry* e = list_iterator_get(it);
		if (!strcmp(tKey, e->mKey)) {
			string_map_remove_element(e);
			list_iterator_remove(&bucket->mEntries, it);
			return;
		}
		
		if (!list_has_next(it)) {
			logError("Unable to find key in map.");
			logErrorString(tKey);
			abortSystem();
		}

		list_iterator_increase(&it);
	}
}

void* string_map_get(StringMap* tMap, char* tKey) {
	int offset = getBucketIDFromString(tKey);
	StringMapBucket* bucket = vector_get(&tMap->mBuckets, offset);

	if (!list_size(&bucket->mEntries)) {
		logError("Unable to find key in map.");
		logErrorString(tKey);
		abortSystem();
	}

	ListIterator it = list_iterator_begin(&bucket->mEntries);
	while (1) {
		StringMapBucketListEntry* e = list_iterator_get(it);
		if (!strcmp(tKey, e->mKey)) {
			return e->mData;
		}
		
		if (!list_has_next(it)) {
			logError("Unable to find key in map.");
			logErrorString(tKey);
			abortSystem();
		}

		list_iterator_increase(&it);
	}

}

typedef struct {
	void* mCaller;
	stringMapMapCB mCB;

} StringMapCaller;

static void string_map_map_single_list_entry(void* tCaller, void* tData) {
	StringMapCaller* caller = tCaller;
	StringMapBucketListEntry* e = tData;

	caller->mCB(caller->mCaller, e->mKey, e->mData);
}

static void string_map_map_single_bucket(void* tCaller, void* tData) {
	StringMapBucket* e = tData;
	list_map(&e->mEntries, string_map_map_single_list_entry, tCaller);

}

void string_map_map(StringMap* tMap, stringMapMapCB tCB, void* tCaller) {
	StringMapCaller caller;
	caller.mCaller = tCaller;
	caller.mCB = tCB;
	vector_map(&tMap->mBuckets, string_map_map_single_bucket, &caller);
}

int string_map_contains(StringMap* tMap, char* tKey) {
	int offset = getBucketIDFromString(tKey);
	StringMapBucket* bucket = vector_get(&tMap->mBuckets, offset);

	if (!list_size(&bucket->mEntries)) return 0;

	ListIterator it = list_iterator_begin(&bucket->mEntries);
	while (1) {
		StringMapBucketListEntry* e = list_iterator_get(it);
		if (!strcmp(tKey, e->mKey)) {
			return 1;
		}

		if (!list_has_next(it)) {
			break;
		}

		list_iterator_increase(&it);
	}

	return 0;
}
