#include "tari/memoryhandler.h"

#include "tari/log.h"
#include "tari/system.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

// TODO: update to newest KOS, so no more manual memory tracking required
extern void initMemoryHandlerHW(); 
extern void increaseAvailableTextureMemoryHW(size_t tSize);
extern void decreaseAvailableTextureMemoryHW(size_t tSize);

#ifdef DREAMCAST

#include <kos.h>

#define allocTextureHW pvr_mem_malloc
#define freeTextureHW pvr_mem_free

#elif defined _WIN32

#include <SDL.h>
#include "tari/texture.h"

void freeSDLTexture(void* tData) {
	SDLTextureData* e = tData;
	SDL_DestroyTexture(e->mTexture);
	SDL_FreeSurface(e->mSurface);
	free(tData);
}

#define allocTextureHW malloc
#define freeTextureHW freeSDLTexture

#endif

static void addToUsageQueueFront(TextureMemory tMem);
static void removeFromUsageQueue(TextureMemory tMem);
static void makeSpaceInTextureMemory(size_t tSize);



static void* allocTextureFunc(size_t tSize) {

	makeSpaceInTextureMemory(tSize);

	TextureMemory ret = malloc(sizeof(struct TextureMemory_internal));
	decreaseAvailableTextureMemoryHW(tSize);
	ret->mData = allocTextureHW(tSize);
	ret->mSize = tSize;
	ret->mIsVirtual = 0;
	addToUsageQueueFront(ret);

	return ret;
}

static void freeTextureFunc(void* tData) {
	TextureMemory tMem = tData;
	removeFromUsageQueue(tMem);
	if(tMem->mIsVirtual) {
		free(tMem->mData);
	} else {
		increaseAvailableTextureMemoryHW(tMem->mSize);
		freeTextureHW(tMem->mData);
	}
	free(tMem);
}


#define MEMORY_STACK_MAX 10

typedef struct MemoryListElement_internal {
	void* mData;
	struct MemoryListElement_internal* mPrev;
	struct MemoryListElement_internal* mNext;
} MemoryListElement;

#define MEMORY_MAP_BUCKET_AMOUNT 101

typedef struct {
	int mSize;
	MemoryListElement* mFirst;
} MemoryMapBucket;

typedef struct {
	MemoryMapBucket mBuckets[MEMORY_MAP_BUCKET_AMOUNT];
} MemoryMap;

typedef struct {
	MemoryMap mMap;
} MemoryList;

typedef struct {
	int mSize;
	TextureMemory mFirst;
	TextureMemory mLast;
} TextureMemoryUsageList;

typedef struct {
	int mHead;
	MemoryList mLists[MEMORY_STACK_MAX];
} MemoryListStack;

static struct {
	MemoryListStack mMemoryStack;
	MemoryListStack mTextureMemoryStack;
	TextureMemoryUsageList mTextureMemoryUsageList;
	int mActive;
} gData;

typedef void* (*MallocFunc)(size_t tSize);
typedef void(*FreeFunc)(void* tData);
typedef void* (*ReallocFunc)(void* tPointer, size_t tSize);

static void printMemoryMapBucket(MemoryMapBucket* tList) {
	if(!tList->mSize) return;

	logInteger(tList->mSize);
	int amount = tList->mSize;

	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		logHex(cur->mData);

		cur = cur->mNext;
	}
}

static void printMemoryMap(MemoryMap* tMap) {
	int i;
	for(i = 0; i < MEMORY_MAP_BUCKET_AMOUNT; i++) {
		printMemoryMapBucket(&tMap->mBuckets[i]);
	}
}

static void printMemoryList(MemoryList* tList) {
	printMemoryMap(&tList->mMap);

}

static void printMemoryListStack(MemoryListStack* tStack) {
	logInteger(tStack->mHead);
	int current;
	for (current = tStack->mHead; current >= 0; current--) {
		logInteger(current);
		printMemoryList(&tStack->mLists[current]);
	}
}

void debugPrintMemoryStack() {
	printMemoryListStack(&gData.mMemoryStack);
}

static void addToUsageQueueFront(TextureMemory tMem) {
	TextureMemory prevFirst = gData.mTextureMemoryUsageList.mFirst;
	if (prevFirst == NULL) {
		gData.mTextureMemoryUsageList.mFirst = gData.mTextureMemoryUsageList.mLast = tMem;
		tMem->mPrevInUsageList = tMem->mNextInUsageList = NULL;
	}
	else {
		gData.mTextureMemoryUsageList.mFirst = tMem;
		prevFirst->mPrevInUsageList = tMem;

		tMem->mPrevInUsageList = NULL;
		tMem->mNextInUsageList = prevFirst;
	}

	gData.mTextureMemoryUsageList.mSize++;
}

static void removeFromUsageQueue(TextureMemory tMem) {
	TextureMemory prev = tMem->mPrevInUsageList;
	TextureMemory next = tMem->mNextInUsageList;

	if (prev != NULL) prev->mNextInUsageList = next;
	else gData.mTextureMemoryUsageList.mFirst = next;

	if (next != NULL) next->mPrevInUsageList = prev;
	else gData.mTextureMemoryUsageList.mLast = prev;

	gData.mTextureMemoryUsageList.mSize--;

	tMem->mNextInUsageList = NULL;
	tMem->mPrevInUsageList = NULL;
}

static void moveTextureMemoryInUsageQueueToFront(TextureMemory tMem) {
	removeFromUsageQueue(tMem);
	addToUsageQueueFront(tMem);
}

static void virtualizeSingleTextureMemory(TextureMemory tMem) {
	debugLog("Virtualizing texture memory.");
	debugInteger(tMem->mSize);

	void* mainMemoryBuffer = malloc(tMem->mSize);
	memcpy(mainMemoryBuffer, tMem->mData, tMem->mSize);
	increaseAvailableTextureMemoryHW(tMem->mSize);
	freeTextureHW(tMem->mData);
	tMem->mData = mainMemoryBuffer;
	tMem->mIsVirtual = 1;
}

static void unvirtualizeSingleTextureMemory(TextureMemory tMem) {
	debugLog("Unvirtualizing texture memory.");
	debugInteger(tMem->mSize);

	decreaseAvailableTextureMemoryHW(tMem->mSize);
	void* textureMemoryBuffer = allocTextureHW(tMem->mSize);
	memcpy(textureMemoryBuffer, tMem->mData, tMem->mSize);
	free(tMem->mData);
	tMem->mData = textureMemoryBuffer;
	tMem->mIsVirtual = 0;

	addToUsageQueueFront(tMem);
}

static void virtualizeTextureMemory(size_t tSize) {
	TextureMemory cur = gData.mTextureMemoryUsageList.mLast;

	int sizeLeft = (int)tSize;
	while (cur != NULL) {
		TextureMemory next = cur->mPrevInUsageList;
		virtualizeSingleTextureMemory(cur);
		removeFromUsageQueue(cur);
		sizeLeft -= cur->mSize;
		cur = next;
		if(sizeLeft <= 0) break;
	}
}

static void makeSpaceInTextureMemory(size_t tSize) {
	if (!gData.mActive) return;
	int available = getAvailableTextureMemory();

	if ((int)tSize <= available) return;

	int needed = tSize - available;

	virtualizeTextureMemory(needed);
}

static void* addMemoryToMemoryMapBucket(MemoryMapBucket* tList, MemoryListElement* e) {
	

	if (tList->mFirst != NULL) tList->mFirst->mPrev = e;
	e->mNext = tList->mFirst;
	tList->mFirst = e;

	tList->mSize++;

	return e->mData;
}

static int getBucketFromPointer(void* tPtr) {
	return (((int)tPtr) % MEMORY_MAP_BUCKET_AMOUNT);
}

static void* addMemoryToMemoryMap(MemoryMap* tMap, int tSize, MallocFunc tFunc) {
	MemoryListElement* e = malloc(sizeof(MemoryListElement));

	e->mPrev = NULL;
	e->mData = tFunc(tSize);

	int whichBucket = getBucketFromPointer(e->mData);
	return addMemoryToMemoryMapBucket(&tMap->mBuckets[whichBucket], e);
}

static void* addAllocatedMemoryToMemoryMap(MemoryMap* tMap, void* tData) {
	MemoryListElement* e = malloc(sizeof(MemoryListElement));

	e->mPrev = NULL;
	e->mData = tData;

	int whichBucket = getBucketFromPointer(e->mData);
	return addMemoryToMemoryMapBucket(&tMap->mBuckets[whichBucket], e);
}

static void* addMemoryToMemoryList(MemoryList* tList, int tSize, MallocFunc tFunc) {
	return addMemoryToMemoryMap(&tList->mMap, tSize, tFunc);
}

static void removeMemoryElementFromMemoryMapBucketWithoutFreeingData(MemoryMapBucket* tList, MemoryListElement* e) {
	if (tList->mFirst == e) tList->mFirst = e->mNext;
	if (e->mNext != NULL) e->mNext->mPrev = e->mPrev;
	if (e->mPrev != NULL) e->mPrev->mNext = e->mNext;
	free(e);

	tList->mSize--;
}

static int removeMemoryFromMemoryMapBucketWithoutFreeingData(MemoryMapBucket* tList, void* tData) {

	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		if (cur->mData == tData) {
			removeMemoryElementFromMemoryMapBucketWithoutFreeingData(tList, cur);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static void removeMemoryElementFromMemoryMapBucket(MemoryMapBucket* tList, MemoryListElement* e, FreeFunc tFunc) {
	tFunc(e->mData);
	removeMemoryElementFromMemoryMapBucketWithoutFreeingData(tList, e);
}

static int removeMemoryFromMemoryMapBucket(MemoryMapBucket* tList, void* tData, FreeFunc tFunc) {

	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		if (cur->mData == tData) {
			removeMemoryElementFromMemoryMapBucket(tList, cur, tFunc);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static int removeMemoryFromMemoryMap(MemoryMap* tMap, void* tData, FreeFunc tFunc) {
	int whichBucket = getBucketFromPointer(tData);
	return removeMemoryFromMemoryMapBucket(&tMap->mBuckets[whichBucket], tData, tFunc);
}

static int removeMemoryFromMemoryList(MemoryList* tList, void* tData, FreeFunc tFunc) {
	return removeMemoryFromMemoryMap(&tList->mMap, tData, tFunc);
}

static void* resizeMemoryElementOnMemoryMapBucket(MemoryListElement* e, int tSize, ReallocFunc tFunc) {
	e->mData = tFunc(e->mData, tSize);
	return e->mData;
}


static int resizeMemoryOnMemoryMapBucket(MemoryMapBucket* tList, void* tData, int tSize, ReallocFunc tFunc, void** tOutput) {
	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		if (cur->mData == tData) {
			*tOutput = resizeMemoryElementOnMemoryMapBucket(cur, tSize, tFunc);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static void moveMemoryInMap(MemoryMap* tMap, void* tDst, void* tSrc) {
	int srcBucket = getBucketFromPointer(tSrc);

	int ret = removeMemoryFromMemoryMapBucketWithoutFreeingData(&tMap->mBuckets[srcBucket], tDst);
	assert(ret);
	addAllocatedMemoryToMemoryMap(tMap, tDst);
}

static int resizeMemoryOnMemoryMap(MemoryMap* tMap, void* tData, int tSize, ReallocFunc tFunc, void** tOutput) {
	int whichBucket = getBucketFromPointer(tData);
	int ret = resizeMemoryOnMemoryMapBucket(&tMap->mBuckets[whichBucket], tData, tSize, tFunc, tOutput);

	int newBucket = getBucketFromPointer(*tOutput);
	if(ret && whichBucket != newBucket) {
		moveMemoryInMap(tMap, *tOutput, tData);
	}

	return ret;
}

static int resizeMemoryOnMemoryList(MemoryList* tList, void* tData, int tSize, ReallocFunc tFunc, void** tOutput) {
	return resizeMemoryOnMemoryMap(&tList->mMap, tData, tSize, tFunc, tOutput);
}

static void emptyMemoryMapBucket(MemoryMapBucket* tList, FreeFunc tFunc) {
	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		MemoryListElement* next = cur->mNext;
		removeMemoryElementFromMemoryMapBucket(tList, cur, tFunc);
		cur = next;
	}
}

static void emptyMemoryMap(MemoryMap* tMap, FreeFunc tFunc) {
	int i;
	for(i = 0; i < MEMORY_MAP_BUCKET_AMOUNT; i++) {
		emptyMemoryMapBucket(&tMap->mBuckets[i], tFunc);
	}
}

static void emptyMemoryList(MemoryList* tList, FreeFunc tFunc) {
	emptyMemoryMap(&tList->mMap, tFunc);
}

static void* addMemoryToMemoryListStack(MemoryListStack* tStack, int tSize, MallocFunc tFunc) {
	if (!gData.mActive) {
		return tFunc(tSize);
	}

	if (tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	if (tSize < 0) {
		logError("Invalid alloc size");
		logErrorInteger(tSize);
		abortSystem();
	}

	return addMemoryToMemoryList(&tStack->mLists[tStack->mHead], tSize, tFunc);
}
static void removeMemoryFromMemoryListStack(MemoryListStack* tStack, void* tData, FreeFunc tFunc) {
	if (!gData.mActive) {
		tFunc(tData);
		return;
	}

	if (tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	int tempHead;
	for (tempHead = tStack->mHead; tempHead >= 0; tempHead--) {
		if (removeMemoryFromMemoryList(&tStack->mLists[tempHead], tData, tFunc)) {
			return;
		}
	}

	logError("Freeing invalid memory address");
	logErrorHex(tData);
	abortSystem();
}

static void* resizeMemoryOnMemoryListStack(MemoryListStack* tStack, void* tData, int tSize, ReallocFunc tFunc) {
	if (!gData.mActive) {
		return tFunc(tData, tSize);
	}

	if (tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	void* output = NULL;
	int tempHead;
	for (tempHead = tStack->mHead; tempHead >= 0; tempHead--) {
		if (resizeMemoryOnMemoryList(&tStack->mLists[tempHead], tData, tSize, tFunc, &output)) {
			return output;
		}
	}

	logError("Reallocating invalid memory address");
	logErrorHex(tData);
	abortSystem();

#ifdef DREAMCAST
	return NULL; // TODO: fix unreachable code (Windows) / no return (DC) conflict
#endif
}

static void popMemoryStackInternal(MemoryListStack* tStack, FreeFunc tFunc) {
	if (tStack->mHead < 0) {
		logError("No stack layer left to pop.");
		abortSystem();
	}

	emptyMemoryList(&tStack->mLists[tStack->mHead], tFunc);
	tStack->mHead--;
}

static void emptyMemoryListStack(MemoryListStack* tStack, FreeFunc tFunc) {
	while (tStack->mHead >= 0) {
		popMemoryStackInternal(tStack, tFunc);
	}
}

void* allocMemory(int tSize) {
	if (!tSize) return NULL;

	return addMemoryToMemoryListStack(&gData.mMemoryStack, tSize, malloc);
}

void freeMemory(void* tData) {
	if (tData == NULL) return;

	removeMemoryFromMemoryListStack(&gData.mMemoryStack, tData, free);
}

void* reallocMemory(void* tData, int tSize) {
	if (!tSize) {
		if (tData != NULL) freeMemory(tData);
		return NULL;
	}
	if (tData == NULL) return allocMemory(tSize);

	return resizeMemoryOnMemoryListStack(&gData.mMemoryStack, tData, tSize, realloc);
}

TextureMemory allocTextureMemory(int tSize) {
	if (!tSize) {
		return NULL;
	}

	return addMemoryToMemoryListStack(&gData.mTextureMemoryStack, tSize, allocTextureFunc);
}

void freeTextureMemory(TextureMemory tMem) {
	if (tMem == NULL) return;

	removeMemoryFromMemoryListStack(&gData.mTextureMemoryStack, tMem, freeTextureFunc);
}

void referenceTextureMemory(TextureMemory tMem) {
	if (!gData.mActive) return;
	if (tMem == NULL) return;

	if (tMem->mIsVirtual) {
		makeSpaceInTextureMemory(tMem->mSize);
		unvirtualizeSingleTextureMemory(tMem);
	}
	moveTextureMemoryInUsageQueueToFront(tMem);
}

static void pushMemoryMapBucketInternal(MemoryMapBucket* tList) {
	tList->mFirst = NULL;
	tList->mSize = 0;
}

static void pushMemoryMapInternal(MemoryMap* tMap) {
	int i;
	for(i = 0; i < MEMORY_MAP_BUCKET_AMOUNT; i++) {
		pushMemoryMapBucketInternal(&tMap->mBuckets[i]);
	}
}

static void pushMemoryStackInternal(MemoryListStack* tStack) {
	if (tStack->mHead == MEMORY_STACK_MAX - 1) {
		logError("Unable to push stack layer; limit reached");
		abortSystem();
	}

	tStack->mHead++;
	pushMemoryMapInternal(&tStack->mLists[tStack->mHead].mMap);
}

void pushMemoryStack() {
	pushMemoryStackInternal(&gData.mMemoryStack);

}
void popMemoryStack() {
	popMemoryStackInternal(&gData.mMemoryStack, free);
}

void pushTextureMemoryStack() {
	pushMemoryStackInternal(&gData.mTextureMemoryStack);
}
void popTextureMemoryStack() {
	popMemoryStackInternal(&gData.mTextureMemoryStack, freeTextureFunc);
}

static void initTextureMemoryUsageList() {
	gData.mTextureMemoryUsageList.mFirst = NULL;
	gData.mTextureMemoryUsageList.mLast = NULL;
	gData.mTextureMemoryUsageList.mSize = 0;
}

void initMemoryHandler() {
	if (gData.mActive) {
		logWarning("Memory Handler was already initialized; Resetting;");
		shutdownMemoryHandler();
	}

	gData.mActive = 1;
	gData.mMemoryStack.mHead = -1;
	gData.mTextureMemoryStack.mHead = -1;
	initTextureMemoryUsageList();
	pushMemoryStack();
	pushTextureMemoryStack();
	initMemoryHandlerHW();
}

void shutdownMemoryHandler() {
	gData.mActive = 0;
	emptyMemoryListStack(&gData.mMemoryStack, free);
	emptyMemoryListStack(&gData.mTextureMemoryStack, freeTextureFunc);
}

