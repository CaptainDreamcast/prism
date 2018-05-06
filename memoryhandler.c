#include "prism/memoryhandler.h"

#include "prism/log.h"
#include "prism/system.h"

#include <zstd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "prism/klib/khash.h"
#include "prism/memorypool.h"

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

KHASH_MAP_INIT_INT(Bucket, void*)

#define MEMORY_STACK_MAX 10

typedef struct {
	//khash_t(Bucket)* mMap;
	void* mData;
} MemoryHandlerMap;

typedef struct {
	int mSize;
	TextureMemory mFirst;
	TextureMemory mLast;
} TextureMemoryUsageList;

typedef void* (*MallocFunc)(size_t tSize);
typedef void(*FreeFunc)(void* tData);
typedef void* (*ReallocFunc)(void* tPointer, size_t tSize);

typedef struct AllocationStrategy_internal {
	void(*mInitMemoryHandlerMap)(struct AllocationStrategy_internal* tStrategy, MemoryHandlerMap* tMap);
	void*(*mAddMemoryToMemoryHandlerMap)(struct AllocationStrategy_internal* tStrategy, MemoryHandlerMap* tMap, int tSize);
	int(*mRemoveMemoryFromMemoryHandlerMap)(struct AllocationStrategy_internal* tStrategy, MemoryHandlerMap* tMap, void* tData);
	int(*mResizeMemoryOnMemoryHandlerMap)(struct AllocationStrategy_internal* tStrategy, MemoryHandlerMap* tMap, void* tData, int tSize, void** tOutput);
	void(*mEmptyMemoryHandlerMap)(struct AllocationStrategy_internal* tStrategy, MemoryHandlerMap* tMap);

	MallocFunc mMalloc;
	FreeFunc mFreeFunc;
	ReallocFunc mReallocFunc;
} AllocationStrategy;

typedef struct {
	int mHead;
	MemoryHandlerMap mMaps[MEMORY_STACK_MAX];
	AllocationStrategy mStrategy;
} MemoryListStack;

static struct {
	MemoryListStack mMemoryStack;
	MemoryListStack mTextureMemoryStack;
	TextureMemoryUsageList mTextureMemoryUsageList;

	int mAllocatedMemory;

	int mIsCompressionActive;

	int mActive;
} gMemoryHandler;






static void initHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap) {
	(void)tStrategy;
	tMap->mData = (void*)kh_init(Bucket);
}

static void* addAllocatedMemoryToMemoryHandlerMapHashMapStrategy(MemoryHandlerMap* tMap, void* tData) {
	khash_t(Bucket)* map = tMap->mData;
	khiter_t iter;
	int ret;
	iter = kh_put(Bucket, map, (khint32_t)tData, &ret);
	kh_val(map, iter) = tData;

	return tData;
}

static void* addMemoryToMemoryHandlerMapHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, int tSize) {
	void* data = tStrategy->mMalloc(tSize);
	gMemoryHandler.mAllocatedMemory++;

	return addAllocatedMemoryToMemoryHandlerMapHashMapStrategy(tMap, data);
}

static int removeMemoryFromMemoryHandlerMapWithoutFreeingMemoryHashMapStrategy(MemoryHandlerMap* tMap, void* tData) {
	khash_t(Bucket)* map = tMap->mData;

	khiter_t iter;
	iter = kh_get(Bucket, map, (khint32_t)tData);
	if (iter == kh_end(map)) {
		return 0;
	}
	kh_del(Bucket, map, iter);

	return 1;
}

static int removeMemoryFromMemoryHandlerMapHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, void* tData) {
	if (!removeMemoryFromMemoryHandlerMapWithoutFreeingMemoryHashMapStrategy(tMap, tData)) {
		return 0;
	}

	gMemoryHandler.mAllocatedMemory--;
	tStrategy->mFreeFunc(tData);

	return 1;
}


static int resizeMemoryOnMemoryHandlerMapHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, void* tData, int tSize, void** tOutput) {
	if (!removeMemoryFromMemoryHandlerMapWithoutFreeingMemoryHashMapStrategy(tMap, tData)) {
		return 0;
	}

	*tOutput = tStrategy->mReallocFunc(tData, tSize);
	addAllocatedMemoryToMemoryHandlerMapHashMapStrategy(tMap, *tOutput);

	return 1;
}

static void emptyMemoryHandlerMapHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap) {
	khash_t(Bucket)* map = tMap->mData;
	khiter_t iter;
	for (iter = kh_begin(map); iter != kh_end(map); ++iter) {
		if (kh_exist(map, iter)) {
			void* data = kh_value(map, iter);
			gMemoryHandler.mAllocatedMemory--;
			tStrategy->mFreeFunc(data);
		}
	}
	kh_destroy(Bucket, map);
}

static AllocationStrategy HashMapStrategyMainMemory = {
	.mInitMemoryHandlerMap = initHashMapStrategy,
	.mAddMemoryToMemoryHandlerMap = addMemoryToMemoryHandlerMapHashMapStrategy,
	.mRemoveMemoryFromMemoryHandlerMap = removeMemoryFromMemoryHandlerMapHashMapStrategy,
	.mResizeMemoryOnMemoryHandlerMap = resizeMemoryOnMemoryHandlerMapHashMapStrategy,
	.mEmptyMemoryHandlerMap = emptyMemoryHandlerMapHashMapStrategy,

	.mMalloc = malloc,
	.mFreeFunc = free,
	.mReallocFunc = realloc,
};

static AllocationStrategy HashMapStrategyTextureMemory = {
	.mInitMemoryHandlerMap = initHashMapStrategy,
	.mAddMemoryToMemoryHandlerMap = addMemoryToMemoryHandlerMapHashMapStrategy,
	.mRemoveMemoryFromMemoryHandlerMap = removeMemoryFromMemoryHandlerMapHashMapStrategy,
	.mResizeMemoryOnMemoryHandlerMap = resizeMemoryOnMemoryHandlerMapHashMapStrategy,
	.mEmptyMemoryHandlerMap = emptyMemoryHandlerMapHashMapStrategy,

	.mMalloc = allocTextureFunc,
	.mFreeFunc = freeTextureFunc,
	.mReallocFunc = NULL,
};

static void initPoolStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap) {
	(void)tStrategy;
	tMap->mData = createMemoryPool(32 * 1000 * 1000);
}

static void* addMemoryToMemoryHandlerMapPoolStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, int tSize) {
	(void)tStrategy;
	return allocPoolMemory(tMap->mData, tSize);
}

static int removeMemoryFromMemoryHandlerMapPoolStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, void* tData) {
	(void)tStrategy;
	if (!isMemoryInPool(tMap->mData, tData)) return 0;

	freePoolMemory(tMap->mData, tData);
	return 1;
}


static int resizeMemoryOnMemoryHandlerMapPoolStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, void* tData, int tSize, void** tOutput) {
	(void)tStrategy;

	if (!isMemoryInPool(tMap->mData, tData)) return 0;

	*tOutput = reallocPoolMemory(tMap->mData, tData, tSize);
	return 1;
}

static void emptyMemoryHandlerMapPoolStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap) {
	(void)tStrategy;
	destroyMemoryPool(tMap->mData);
}

static AllocationStrategy PoolStrategyMainMemory = {
	.mInitMemoryHandlerMap = initPoolStrategy,
	.mAddMemoryToMemoryHandlerMap = addMemoryToMemoryHandlerMapPoolStrategy,
	.mRemoveMemoryFromMemoryHandlerMap = removeMemoryFromMemoryHandlerMapPoolStrategy,
	.mResizeMemoryOnMemoryHandlerMap = resizeMemoryOnMemoryHandlerMapPoolStrategy,
	.mEmptyMemoryHandlerMap = emptyMemoryHandlerMapPoolStrategy,

	.mMalloc = malloc,
	.mFreeFunc = free,
	.mReallocFunc = realloc,
};


int getAllocatedMemoryBlockAmount() {
	return gMemoryHandler.mAllocatedMemory;
}



void debugPrintMemoryStack() {
	logError("Unimplemented.");
	abortSystem();
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

	char* src = *tBuffer;
	int dstBufferSize = tSrcSize + COMPRESSION_BUFFER;
	char* dst = malloc(dstBufferSize);
	int dstLength = ZSTD_compress(dst, dstBufferSize, src, tSrcSize, 1);
	dst = realloc(dst, dstLength);

	free(src);
	*tBuffer = dst;
	tMem->mIsCompressed = 1;
	tMem->mCompressedSize = dstLength;
}

static void decompressMemory(TextureMemory tMem, void** tBuffer) {
	if (!tMem->mIsCompressed) {
		return;
	}
	
	char* src = *tBuffer;
	size_t uncompressedLength = (size_t)ZSTD_getFrameContentSize(src, tMem->mCompressedSize);

	char* dst = malloc(uncompressedLength);
	int dstLength = ZSTD_decompress(dst, uncompressedLength, src, tMem->mCompressedSize);
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

static void* addMemoryToMemoryListStack(MemoryListStack* tStack, int tSize) {
	if (!gMemoryHandler.mActive) {
		return tStack->mStrategy.mMalloc(tSize);
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

	return tStack->mStrategy.mAddMemoryToMemoryHandlerMap(&tStack->mStrategy, &tStack->mMaps[tStack->mHead], tSize);
}
static void removeMemoryFromMemoryListStack(MemoryListStack* tStack, void* tData) {
	if (!gMemoryHandler.mActive) {
		tStack->mStrategy.mFreeFunc(tData);
		return;
	}

	if (tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	int tempHead;
	for (tempHead = tStack->mHead; tempHead >= 0; tempHead--) {
		int hasBeenRemoved = tStack->mStrategy.mRemoveMemoryFromMemoryHandlerMap(&tStack->mStrategy, &tStack->mMaps[tempHead], tData);
		if (hasBeenRemoved) {
			return;
		}
	}

	logError("Freeing invalid memory address");
	logErrorHex(tData);
	abortSystem();
}

static void* resizeMemoryOnMemoryListStack(MemoryListStack* tStack, void* tData, int tSize) {
	if (!gMemoryHandler.mActive) {
		return tStack->mStrategy.mReallocFunc(tData, tSize);
	}

	if (tStack->mHead < 0) {
		logError("Invalid stack head position");
		logErrorInteger(tStack->mHead);
		abortSystem();
	}

	void* output = NULL;
	int tempHead;
	for (tempHead = tStack->mHead; tempHead >= 0; tempHead--) {
		int hasBeenResized = tStack->mStrategy.mResizeMemoryOnMemoryHandlerMap(&tStack->mStrategy, &tStack->mMaps[tempHead], tData, tSize, &output);
		if (hasBeenResized) {
			return output;
		}
	}

	logError("Reallocating invalid memory address");
	logErrorHex(tData);
	abortSystem();

	return NULL; 
}

static void popMemoryStackInternal(MemoryListStack* tStack) {
	if (tStack->mHead < 0) {
		logError("No stack layer left to pop.");
		abortSystem();
	}

	tStack->mStrategy.mEmptyMemoryHandlerMap(&tStack->mStrategy, &tStack->mMaps[tStack->mHead]);
	tStack->mHead--;
}

static void emptyMemoryListStack(MemoryListStack* tStack) {
	while (tStack->mHead >= 0) {
		popMemoryStackInternal(tStack);
	}
}

void* allocMemory(int tSize) {
	if (!tSize) return NULL;
	return addMemoryToMemoryListStack(&gMemoryHandler.mMemoryStack, tSize);
}

void* allocClearedMemory(int tBlockAmount, int tBlockSize) {
	uint32_t length = tBlockAmount*tBlockSize;

	void* data = allocMemory(length);

	if (data) {
		memset(data, 0, length);
	}

	return data;
}

void freeMemory(void* tData) {
	if (tData == NULL) return;

	removeMemoryFromMemoryListStack(&gMemoryHandler.mMemoryStack, tData);
}

void* reallocMemory(void* tData, int tSize) {
	if (!tSize) {
		if (tData != NULL) freeMemory(tData);
		return NULL;
	}
	if (tData == NULL) return allocMemory(tSize);

	return resizeMemoryOnMemoryListStack(&gMemoryHandler.mMemoryStack, tData, tSize);
}

TextureMemory allocTextureMemory(int tSize) {
	if (!tSize) {
		return NULL;
	}
	
	return addMemoryToMemoryListStack(&gMemoryHandler.mTextureMemoryStack, tSize);
}

void freeTextureMemory(TextureMemory tMem) {
	if (tMem == NULL) return;

	removeMemoryFromMemoryListStack(&gMemoryHandler.mTextureMemoryStack, tMem);
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

static void pushMemoryStackInternal(MemoryListStack* tStack) {
	if (tStack->mHead == MEMORY_STACK_MAX - 1) {
		logError("Unable to push stack layer; limit reached");
		abortSystem();
	}

	tStack->mHead++;
	tStack->mStrategy.mInitMemoryHandlerMap(&tStack->mStrategy, &tStack->mMaps[tStack->mHead]);
}

void pushMemoryStack() {
	pushMemoryStackInternal(&gMemoryHandler.mMemoryStack);

}
void popMemoryStack() {
	popMemoryStackInternal(&gMemoryHandler.mMemoryStack);
}

void pushTextureMemoryStack() {
	pushMemoryStackInternal(&gMemoryHandler.mTextureMemoryStack);
}
void popTextureMemoryStack() {
	popMemoryStackInternal(&gMemoryHandler.mTextureMemoryStack);
}

static void initTextureMemoryUsageList() {
	gMemoryHandler.mTextureMemoryUsageList.mFirst = NULL;
	gMemoryHandler.mTextureMemoryUsageList.mLast = NULL;
	gMemoryHandler.mTextureMemoryUsageList.mSize = 0;
}

static void initMemoryListStack(MemoryListStack* tStack, AllocationStrategy tStrategy) {
	tStack->mHead = -1;
	tStack->mStrategy = tStrategy;
}

void initMemoryHandler() {
	if (gMemoryHandler.mActive) {
		logWarning("Memory Handler was already initialized; Resetting;");
		shutdownMemoryHandler();
	}

	gMemoryHandler.mActive = 1;
	gMemoryHandler.mIsCompressionActive = 0;
	gMemoryHandler.mMemoryStack.mHead = -1;
	initMemoryListStack(&gMemoryHandler.mMemoryStack, PoolStrategyMainMemory);
	initMemoryListStack(&gMemoryHandler.mTextureMemoryStack, HashMapStrategyTextureMemory);
	initTextureMemoryUsageList();
	pushMemoryStack();
	pushTextureMemoryStack();
	initMemoryHandlerHW();
}

void shutdownMemoryHandler() {
	gMemoryHandler.mActive = 0;
	emptyMemoryListStack(&gMemoryHandler.mMemoryStack);
	emptyMemoryListStack(&gMemoryHandler.mTextureMemoryStack);
}


void setMemoryHandlerCompressionActive() {
	gMemoryHandler.mIsCompressionActive = 1;
}

void setMemoryHandlerCompressionInactive() {
	gMemoryHandler.mIsCompressionActive = 0;
}
