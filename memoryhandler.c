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

#define MEMORY_MAP_BUCKET_AMOUNT 1009

typedef struct {
	int mSize;
	MemoryListElement* mFirst;
} MemoryHandlerMapBucket;

typedef struct {
	int mSize;
	MemoryHandlerMapBucket mBuckets[MEMORY_MAP_BUCKET_AMOUNT];
} MemoryHandlerMap;

typedef struct {
	int mSize;
	TextureMemory mFirst;
	TextureMemory mLast;
} TextureMemoryUsageList;

typedef struct {
	int mHead;
	MemoryHandlerMap mMaps[MEMORY_STACK_MAX];
} MemoryListStack;

static struct {
	MemoryListStack mMemoryStack;
	MemoryListStack mTextureMemoryStack;
	TextureMemoryUsageList mTextureMemoryUsageList;
	int mActive;
} gMemoryHandler;

typedef void* (*MallocFunc)(size_t tSize);
typedef void(*FreeFunc)(void* tData);
typedef void* (*ReallocFunc)(void* tPointer, size_t tSize);

static void printMemoryHandlerMapBucket(MemoryHandlerMapBucket* tList) {
	if(!tList->mSize) return;

	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		logHex(cur->mData);

		cur = cur->mNext;
	}
}

static void printMemoryHandlerMap(MemoryHandlerMap* tMap) {
	logInteger(tMap->mSize);
	int i;
	for(i = 0; i < MEMORY_MAP_BUCKET_AMOUNT; i++) {
		printMemoryHandlerMapBucket(&tMap->mBuckets[i]);
	}
}

static void printMemoryListStack(MemoryListStack* tStack) {
	logInteger(tStack->mHead);
	int current;
	for (current = tStack->mHead; current >= 0; current--) {
		logInteger(current);
		logPointer(&tStack->mMaps[current]);
		printMemoryHandlerMap(&tStack->mMaps[current]);
	}
}

void debugPrintMemoryStack() {
	printMemoryListStack(&gMemoryHandler.mMemoryStack);
}

static void addToUsageQueueFront(TextureMemory tMem) {
	TextureMemory prevFirst = gMemoryHandler.mTextureMemoryUsageList.mFirst;
	if (prevFirst == NULL) {
		gMemoryHandler.mTextureMemoryUsageList.mFirst = gMemoryHandler.mTextureMemoryUsageList.mLast = tMem;
		tMem->mPrevInUsageList = tMem->mNextInUsageList = NULL;
	}
	else {
		gMemoryHandler.mTextureMemoryUsageList.mFirst = tMem;
		prevFirst->mPrevInUsageList = tMem;

		tMem->mPrevInUsageList = NULL;
		tMem->mNextInUsageList = prevFirst;
	}

	gMemoryHandler.mTextureMemoryUsageList.mSize++;
}

static void removeFromUsageQueue(TextureMemory tMem) {
	TextureMemory prev = tMem->mPrevInUsageList;
	TextureMemory next = tMem->mNextInUsageList;

	if (prev != NULL) prev->mNextInUsageList = next;
	else gMemoryHandler.mTextureMemoryUsageList.mFirst = next;

	if (next != NULL) next->mPrevInUsageList = prev;
	else gMemoryHandler.mTextureMemoryUsageList.mLast = prev;

	gMemoryHandler.mTextureMemoryUsageList.mSize--;

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
	TextureMemory cur = gMemoryHandler.mTextureMemoryUsageList.mLast;

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
	if (!gMemoryHandler.mActive) return;
	int available = getAvailableTextureMemory();

	if ((int)tSize <= available) return;

	int needed = tSize - available;

	virtualizeTextureMemory(needed);
}

static void* addMemoryToMemoryHandlerMapBucket(MemoryHandlerMapBucket* tList, MemoryListElement* e) {

	if (tList->mFirst != NULL) tList->mFirst->mPrev = e;
	e->mNext = tList->mFirst;
	tList->mFirst = e;

	tList->mSize++;

	return e->mData;
}

static int getBucketFromPointer(void* tPtr) { 
	return (((unsigned int)tPtr) % MEMORY_MAP_BUCKET_AMOUNT);
}

static void* addMemoryToMemoryHandlerMap(MemoryHandlerMap* tMap, int tSize, MallocFunc tFunc) {
	tMap->mSize++;

	MemoryListElement* e = malloc(sizeof(MemoryListElement));

	e->mPrev = NULL;
	e->mData = tFunc(tSize);
	
	int whichBucket = getBucketFromPointer(e->mData);
	return addMemoryToMemoryHandlerMapBucket(&tMap->mBuckets[whichBucket], e);
}

static void* addAllocatedMemoryToMemoryHandlerMap(MemoryHandlerMap* tMap, void* tData) {
	MemoryListElement* e = malloc(sizeof(MemoryListElement));

	e->mPrev = NULL;
	e->mData = tData;

	int whichBucket = getBucketFromPointer(e->mData);
	return addMemoryToMemoryHandlerMapBucket(&tMap->mBuckets[whichBucket], e);
}

static void removeMemoryElementFromMemoryHandlerMapBucketWithoutFreeingMemoryHandler(MemoryHandlerMapBucket* tList, MemoryListElement* e) {
	if (tList->mFirst == e) tList->mFirst = e->mNext;
	if (e->mNext != NULL) e->mNext->mPrev = e->mPrev;
	if (e->mPrev != NULL) e->mPrev->mNext = e->mNext;
	free(e);

	tList->mSize--;
}

static int removeMemoryFromMemoryHandlerMapBucketWithoutFreeingMemoryHandler(MemoryHandlerMapBucket* tList, void* tData) {

	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		if (cur->mData == tData) {
			removeMemoryElementFromMemoryHandlerMapBucketWithoutFreeingMemoryHandler(tList, cur);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static void removeMemoryElementFromMemoryHandlerMapBucket(MemoryHandlerMapBucket* tList, MemoryListElement* e, FreeFunc tFunc) {
	tFunc(e->mData);
	removeMemoryElementFromMemoryHandlerMapBucketWithoutFreeingMemoryHandler(tList, e);
}

static int removeMemoryFromMemoryHandlerMapBucket(MemoryHandlerMapBucket* tList, void* tData, FreeFunc tFunc) {

	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		if (cur->mData == tData) {
			removeMemoryElementFromMemoryHandlerMapBucket(tList, cur, tFunc);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static int removeMemoryFromMemoryHandlerMap(MemoryHandlerMap* tMap, void* tData, FreeFunc tFunc) {
	tMap->mSize--;
	int whichBucket = getBucketFromPointer(tData);
	return removeMemoryFromMemoryHandlerMapBucket(&tMap->mBuckets[whichBucket], tData, tFunc);
}

static void* resizeMemoryElementOnMemoryHandlerMapBucket(MemoryListElement* e, int tSize, ReallocFunc tFunc) {
	e->mData = tFunc(e->mData, tSize);
	return e->mData;
}


static int resizeMemoryOnMemoryHandlerMapBucket(MemoryHandlerMapBucket* tList, void* tData, int tSize, ReallocFunc tFunc, void** tOutput) {
	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		if (cur->mData == tData) {
			*tOutput = resizeMemoryElementOnMemoryHandlerMapBucket(cur, tSize, tFunc);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static void moveMemoryInMap(MemoryHandlerMap* tMap, void* tDst, void* tSrc) {
	int srcBucket = getBucketFromPointer(tSrc);

	int ret = removeMemoryFromMemoryHandlerMapBucketWithoutFreeingMemoryHandler(&tMap->mBuckets[srcBucket], tDst);
	assert(ret);
	addAllocatedMemoryToMemoryHandlerMap(tMap, tDst);
}

static int resizeMemoryOnMemoryHandlerMap(MemoryHandlerMap* tMap, void* tData, int tSize, ReallocFunc tFunc, void** tOutput) {
	int whichBucket = getBucketFromPointer(tData);
	int ret = resizeMemoryOnMemoryHandlerMapBucket(&tMap->mBuckets[whichBucket], tData, tSize, tFunc, tOutput);

	int newBucket = getBucketFromPointer(*tOutput);
	if(ret && whichBucket != newBucket) {
		moveMemoryInMap(tMap, *tOutput, tData);
	}

	return ret;
}

static void emptyMemoryHandlerMapBucket(MemoryHandlerMapBucket* tList, FreeFunc tFunc) {
	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while (amount--) {
		MemoryListElement* next = cur->mNext;
		removeMemoryElementFromMemoryHandlerMapBucket(tList, cur, tFunc);
		cur = next;
	}
}

static void emptyMemoryHandlerMap(MemoryHandlerMap* tMap, FreeFunc tFunc) {
	int i;
	for(i = 0; i < MEMORY_MAP_BUCKET_AMOUNT; i++) {
		emptyMemoryHandlerMapBucket(&tMap->mBuckets[i], tFunc);
	}
}

static void* addMemoryToMemoryListStack(MemoryListStack* tStack, int tSize, MallocFunc tFunc) {
	if (!gMemoryHandler.mActive) {
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

	return addMemoryToMemoryHandlerMap(&tStack->mMaps[tStack->mHead], tSize, tFunc);
}
static void removeMemoryFromMemoryListStack(MemoryListStack* tStack, void* tData, FreeFunc tFunc) {
	if (!gMemoryHandler.mActive) {
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
		if (removeMemoryFromMemoryHandlerMap(&tStack->mMaps[tempHead], tData, tFunc)) {
			return;
		}
	}

	logError("Freeing invalid memory address");
	logErrorHex(tData);
	abortSystem();
}

static void* resizeMemoryOnMemoryListStack(MemoryListStack* tStack, void* tData, int tSize, ReallocFunc tFunc) {
	if (!gMemoryHandler.mActive) {
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
		if (resizeMemoryOnMemoryHandlerMap(&tStack->mMaps[tempHead], tData, tSize, tFunc, &output)) {
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

	emptyMemoryHandlerMap(&tStack->mMaps[tStack->mHead], tFunc);
	tStack->mHead--;
}

static void emptyMemoryListStack(MemoryListStack* tStack, FreeFunc tFunc) {
	while (tStack->mHead >= 0) {
		popMemoryStackInternal(tStack, tFunc);
	}
}

void* allocMemory(int tSize) {
	if (!tSize) return NULL;

	return addMemoryToMemoryListStack(&gMemoryHandler.mMemoryStack, tSize, malloc);
}

void freeMemory(void* tData) {
	if (tData == NULL) return;

	removeMemoryFromMemoryListStack(&gMemoryHandler.mMemoryStack, tData, free);
}

void* reallocMemory(void* tData, int tSize) {
	if (!tSize) {
		if (tData != NULL) freeMemory(tData);
		return NULL;
	}
	if (tData == NULL) return allocMemory(tSize);

	return resizeMemoryOnMemoryListStack(&gMemoryHandler.mMemoryStack, tData, tSize, realloc);
}

TextureMemory allocTextureMemory(int tSize) {
	if (!tSize) {
		return NULL;
	}
	
	return addMemoryToMemoryListStack(&gMemoryHandler.mTextureMemoryStack, tSize, allocTextureFunc);
}

void freeTextureMemory(TextureMemory tMem) {
	if (tMem == NULL) return;

	removeMemoryFromMemoryListStack(&gMemoryHandler.mTextureMemoryStack, tMem, freeTextureFunc);
}

void referenceTextureMemory(TextureMemory tMem) {
	if (!gMemoryHandler.mActive) return;
	if (tMem == NULL) return;

	if (tMem->mIsVirtual) {
		makeSpaceInTextureMemory(tMem->mSize);
		unvirtualizeSingleTextureMemory(tMem);
	}
	moveTextureMemoryInUsageQueueToFront(tMem);
}

static void pushMemoryHandlerMapBucketInternal(MemoryHandlerMapBucket* tList) {
	tList->mFirst = NULL;
	tList->mSize = 0;
}

static void pushMemoryHandlerMapInternal(MemoryHandlerMap* tMap) {
	tMap->mSize = 0;
	int i;
	for(i = 0; i < MEMORY_MAP_BUCKET_AMOUNT; i++) {
		pushMemoryHandlerMapBucketInternal(&tMap->mBuckets[i]);
	}
}

static void pushMemoryStackInternal(MemoryListStack* tStack) {
	if (tStack->mHead == MEMORY_STACK_MAX - 1) {
		logError("Unable to push stack layer; limit reached");
		abortSystem();
	}

	tStack->mHead++;
	pushMemoryHandlerMapInternal(&tStack->mMaps[tStack->mHead]);
}

void pushMemoryStack() {
	pushMemoryStackInternal(&gMemoryHandler.mMemoryStack);

}
void popMemoryStack() {
	popMemoryStackInternal(&gMemoryHandler.mMemoryStack, free);
}

void pushTextureMemoryStack() {
	pushMemoryStackInternal(&gMemoryHandler.mTextureMemoryStack);
}
void popTextureMemoryStack() {
	popMemoryStackInternal(&gMemoryHandler.mTextureMemoryStack, freeTextureFunc);
}

static void initTextureMemoryUsageList() {
	gMemoryHandler.mTextureMemoryUsageList.mFirst = NULL;
	gMemoryHandler.mTextureMemoryUsageList.mLast = NULL;
	gMemoryHandler.mTextureMemoryUsageList.mSize = 0;
}

void initMemoryHandler() {
	if (gMemoryHandler.mActive) {
		logWarning("Memory Handler was already initialized; Resetting;");
		shutdownMemoryHandler();
	}

	gMemoryHandler.mActive = 1;
	gMemoryHandler.mMemoryStack.mHead = -1;
	gMemoryHandler.mTextureMemoryStack.mHead = -1;
	initTextureMemoryUsageList();
	pushMemoryStack();
	pushTextureMemoryStack();
	initMemoryHandlerHW();
}

void shutdownMemoryHandler() {
	gMemoryHandler.mActive = 0;
	emptyMemoryListStack(&gMemoryHandler.mMemoryStack, free);
	emptyMemoryListStack(&gMemoryHandler.mTextureMemoryStack, freeTextureFunc);
}

