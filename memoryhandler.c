#include "prism/memoryhandler.h"

#include "prism/log.h"
#include "prism/system.h"
#include "prism/quicklz.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "prism/klib/khash.h"

// TODO: update to newest KOS, so no more manual memory tracking required
extern void initMemoryHandlerHW(); 
extern void increaseAvailableTextureMemoryHW(size_t tSize);
extern void decreaseAvailableTextureMemoryHW(size_t tSize);

#ifdef DREAMCAST

#include <kos.h>

#define allocTextureHW pvr_mem_malloc
#define freeTextureHW pvr_mem_free

#elif defined _WIN32 || defined __EMSCRIPTEN__

#include <SDL.h>
#include <GL/glew.h>
#include "prism/texture.h"

void freeSDLTexture(void* tData) {
	SDLTextureData* e = tData;
	glDeleteTextures(1, &e->mTexture);
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
	ret->mIsCompressed = 0;
	addToUsageQueueFront(ret);

	return ret;
}

static void freeTextureFunc(void* tData) {
	TextureMemory tMem = tData;
	if(tMem->mIsVirtual) {
		free(tMem->mData);
	} else {
		removeFromUsageQueue(tMem);
		increaseAvailableTextureMemoryHW(tMem->mSize);
		freeTextureHW(tMem->mData);
	}
	free(tMem);
}

KHASH_MAP_INIT_INT64(Bucket, void*)

#define MEMORY_STACK_MAX 10

typedef struct {
	int m;
	khash_t(Bucket)* mMap;
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

	int mIsCompressionActive;

	int mActive;
} gMemoryHandler;

typedef void* (*MallocFunc)(size_t tSize);
typedef void(*FreeFunc)(void* tData);
typedef void* (*ReallocFunc)(void* tPointer, size_t tSize);

static void printMemoryHandlerMap(MemoryHandlerMap* tMap) {
	// TODO
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

static const int COMPRESSION_BUFFER = 400;

static void compressMemory(TextureMemory tMem, void** tBuffer, int tSrcSize) {
	if (!gMemoryHandler.mIsCompressionActive) {
		tMem->mIsCompressed = 0;
		return;
	}

	qlz_state_compress state_compress;
	
	char* src = *tBuffer;
	char* dst = malloc(tSrcSize + COMPRESSION_BUFFER);
	int dstLength = qlz_compress(src, dst, tSrcSize, &state_compress);
	dst = realloc(dst, dstLength);

	free(src);
	*tBuffer = dst;
	tMem->mIsCompressed = 1;
}

static void decompressMemory(TextureMemory tMem, void** tBuffer) {
	if (!tMem->mIsCompressed) {
		return;
	}

	qlz_state_decompress state_decompress;
	
	char* src = *tBuffer;
	size_t uncompressedLength = qlz_size_decompressed(src);

	char* dst = malloc(uncompressedLength);
	int dstLength = qlz_decompress(src, dst, &state_decompress);
	dst = realloc(dst, dstLength);

	free(src);
	*tBuffer = dst;	
}

static void virtualizeSingleTextureMemory(TextureMemory tMem) {
	debugLog("Virtualizing texture memory.");
	debugInteger(tMem->mSize);
	increaseAvailableTextureMemoryHW(tMem->mSize);

	void* mainMemoryBuffer = malloc(tMem->mSize);
	memcpy(mainMemoryBuffer, tMem->mData, tMem->mSize);

	compressMemory(tMem, &mainMemoryBuffer, tMem->mSize);

	freeTextureHW(tMem->mData);
	tMem->mData = mainMemoryBuffer;

	tMem->mIsVirtual = 1;
}

static void unvirtualizeSingleTextureMemory(TextureMemory tMem) {
	debugLog("Unvirtualizing texture memory.");
	debugInteger(tMem->mSize);
  
	decompressMemory(tMem, &tMem->mData);

	void* textureMemoryBuffer = allocTextureHW(tMem->mSize);
	memcpy(textureMemoryBuffer, tMem->mData, tMem->mSize);
	free(tMem->mData);
	tMem->mData = textureMemoryBuffer;
	tMem->mIsVirtual = 0;

	decreaseAvailableTextureMemoryHW(tMem->mSize);

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

	if(sizeLeft > 0) {
		logError("Unable to virtualize enough space for texture.");
		logErrorInteger(tSize);
		logErrorInteger(sizeLeft);
		abortSystem();
	}
}

static void makeSpaceInTextureMemory(size_t tSize) {
	if (!gMemoryHandler.mActive) return;
	int available = getAvailableTextureMemory();

	if ((int)tSize <= available) return;

	int needed = tSize - available;

	virtualizeTextureMemory(needed);
}

static void* addAllocatedMemoryToMemoryHandlerMap(MemoryHandlerMap* tMap, void* tData) {
	khiter_t iter;
	int ret;
	iter = kh_put(Bucket, tMap->mMap, (khint64_t)tData, &ret);
	kh_val(tMap->mMap, iter) = tData;


	return tData;
}


static void* addMemoryToMemoryHandlerMap(MemoryHandlerMap* tMap, int tSize, MallocFunc tFunc) {
	void* data =  tFunc(tSize);
	
	return addAllocatedMemoryToMemoryHandlerMap(tMap, data);
}

static int removeMemoryFromMemoryHandlerMapWithoutFreeingMemory(MemoryHandlerMap* tMap, void* tData) {
	khiter_t iter;
	iter = kh_get(Bucket, tMap->mMap, (khint64_t)tData);
	if (iter == kh_end(tMap->mMap)) {
		return 0;
	}
	kh_del(Bucket, tMap->mMap, iter);

	return 1;
}

static int removeMemoryFromMemoryHandlerMap(MemoryHandlerMap* tMap, void* tData, FreeFunc tFunc) {
	if (!removeMemoryFromMemoryHandlerMapWithoutFreeingMemory(tMap, tData)) {
		return 0;
	}

	tFunc(tData);	

	return 1;
}

static int resizeMemoryOnMemoryHandlerMap(MemoryHandlerMap* tMap, void* tData, int tSize, ReallocFunc tFunc, void** tOutput) {
	if (!removeMemoryFromMemoryHandlerMapWithoutFreeingMemory(tMap, tData)) {
		return 0;
	}

	*tOutput = tFunc(tData, tSize);
	addAllocatedMemoryToMemoryHandlerMap(tMap, *tOutput);

	return 1;
}

static void emptyMemoryHandlerMap(MemoryHandlerMap* tMap, FreeFunc tFunc) {
	khiter_t iter;
	for (iter = kh_begin(tMap->mMap); iter != kh_end(tMap->mMap); ++iter) {
		if (kh_exist(tMap->mMap, iter)) {
			void* data = kh_value(tMap->mMap, iter);
			tFunc(data);
		}
	}
	kh_destroy(Bucket, tMap->mMap);
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

void* allocClearedMemory(int tSize) {
	if (!tSize) return NULL;

	return addMemoryToMemoryListStack(&gMemoryHandler.mMemoryStack, tSize, calloc);
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

static void pushMemoryHandlerMapInternal(MemoryHandlerMap* tMap) {
	tMap->mMap = kh_init(Bucket);
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
	gMemoryHandler.mIsCompressionActive = 0;
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


void setMemoryHandlerCompressionActive() {
	gMemoryHandler.mIsCompressionActive = 1;
}

void setMemoryHandlerCompressionInactive() {
	gMemoryHandler.mIsCompressionActive = 0;
}
