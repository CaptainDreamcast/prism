#include "include/memoryhandler.h"

#include "include/log.h"
#include "include/system.h"

#define MEMORY_STACK_MAX 10

typedef struct MemoryListElement_internal{
	void* mData;
	struct MemoryListElement_internal* mPrev;
	struct MemoryListElement_internal* mNext;
} MemoryListElement;

typedef struct {
	int mSize;
	MemoryListElement* mFirst;
} MemoryList;

typedef struct {
	int mHead;
	MemoryList mLists[MEMORY_STACK_MAX];
} MemoryListStack;

static struct {
	MemoryListStack mMemoryStack;
	MemoryListStack mTextureMemoryStack;
	int mActive;
} gData;


typedef void* (*MallocFunc)(size_t tSize);
typedef void (*FreeFunc)(void* tSize);


static void* addMemoryToMemoryList(MemoryList* tList, int tSize, MallocFunc tFunc) {
	MemoryListElement* e = malloc(sizeof(MemoryListElement));

	e->mPrev = NULL;
	e->mData = tFunc(tSize);
	
	if(tList->mFirst != NULL) tList->mFirst->mPrev = e;
	e->mNext = tList->mFirst;
	tList->mFirst = e;

	tList->mSize++;

	return e->mData;
}

static void removeMemoryElementFromMemoryList(MemoryList* tList, MemoryListElement* e, FreeFunc tFunc) {
	if(tList->mFirst == e) tList->mFirst = e->mNext;
	if(e->mNext != NULL) e->mNext->mPrev = e->mPrev;
	if(e->mPrev != NULL) e->mPrev->mNext = e->mNext;
	tFunc(e->mData);
	free(e);	

	tList->mSize--;
}

static int removeMemoryFromMemoryList(MemoryList* tList, void* tData, FreeFunc tFunc) {
	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while(amount--) {
		if(cur->mData == tData) {
			removeMemoryElementFromMemoryList(tList, cur, tFunc);
			return 1;
		}

		cur = cur->mNext;
	}

	return 0;
}

static void emptyMemoryList(MemoryList* tList, FreeFunc tFunc) {
	int amount = tList->mSize;
	MemoryListElement* cur = tList->mFirst;
	while(amount--) {
		MemoryListElement* next = cur->mNext;
		removeMemoryElementFromMemoryList(tList, cur, tFunc);
		cur = next;
	}
}

static void* addMemoryToMemoryListStack(MemoryListStack* tStack, int tSize, MallocFunc tFunc) {
	if(!gData.mActive) {
		return tFunc(tSize);
	}

	if(tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	if(tSize < 0) {
		logError("Invalid alloc size");
		logErrorInteger(tSize);
		abortSystem();
	}

	return addMemoryToMemoryList(&tStack->mLists[tStack->mHead], tSize, tFunc);
}
static void removeMemoryFromMemoryListStack(MemoryListStack* tStack, void* tData, FreeFunc tFunc) {
	if(!gData.mActive) {
		tFunc(tData);
		return;
	}

	if(tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	int tempHead = tStack->mHead;
	while(tempHead-- >= 0) {
		if(removeMemoryFromMemoryList(&tStack->mLists[tempHead], tData, tFunc)) {
			return;
		}
	}

	logError("Freeing invalid memory address");
	logErrorHex(tData);
	abortSystem();
}

static void popMemoryStackInternal(MemoryListStack* tStack, FreeFunc tFunc) {
	if(tStack->mHead < 0) {
		logError("No stack layer left to pop.");
		abortSystem();
	}

	emptyMemoryList(&tStack->mLists[tStack->mHead], tFunc);	
	tStack->mHead--;
}

static void emptyMemoryListStack(MemoryListStack* tStack, FreeFunc tFunc) {
	while(tStack->mHead >= 0) {
		popMemoryStackInternal(tStack, tFunc);
	}
}

void* allocMemory(int tSize) {
	if(!tSize) return NULL;

	return addMemoryToMemoryListStack(&gData.mMemoryStack, tSize, malloc);
}
void freeMemory(void* tData) {
	if(tData == NULL) return;

	removeMemoryFromMemoryListStack(&gData.mMemoryStack, tData, free);
}
void* allocTextureMemory(int tSize)  {
	if(!tSize) return NULL;

	return addMemoryToMemoryListStack(&gData.mTextureMemoryStack, tSize, pvr_mem_malloc);
}
void freeTextureMemory(void* tData) {
	if(tData == NULL) return;

	removeMemoryFromMemoryListStack(&gData.mTextureMemoryStack, tData, pvr_mem_free);
}

static void pushMemoryStackInternal(MemoryListStack* tStack) {
	if(tStack->mHead == MEMORY_STACK_MAX - 1) {
		logError("Unable to push stack layer; limit reached");
		abortSystem();
	}

	tStack->mHead++;
	tStack->mLists[tStack->mHead].mFirst = NULL;
	tStack->mLists[tStack->mHead].mSize = 0;
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
	popMemoryStackInternal(&gData.mTextureMemoryStack, pvr_mem_free);
}

void initMemoryHandler() {
	if(gData.mActive) {
		logWarning("Memory Handler was already initialized; Resetting;");
		shutdownMemoryHandler();
	}

	gData.mActive = 1;
	gData.mMemoryStack.mHead = -1;	
	gData.mTextureMemoryStack.mHead = -1;
	pushTextureMemoryStack();
}

void shutdownMemoryHandler() {
	gData.mActive = 0;
	emptyMemoryListStack(&gData.mMemoryStack, free);
	emptyMemoryListStack(&gData.mTextureMemoryStack, pvr_mem_free);
}

