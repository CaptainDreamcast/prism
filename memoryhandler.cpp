#include "prism/memoryhandler.h"

#include "prism/log.h"
#include "prism/system.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "prism/stlutil.h"
#include "prism/memorypool.h"

using namespace std;

static void compressMemory(TextureMemory tMem, void** tBuffer, int tSrcSize);
static void decompressMemory(TextureMemory tMem, void** tBuffer);

#ifdef DREAMCAST

#include <kos.h>
#include <zstd.h>

#define allocTextureHW pvr_mem_malloc
#define freeTextureHW pvr_mem_free

void virtualizeTextureDreamcast(const TextureMemory& tMem)
{
	void* mainMemoryBuffer = malloc(tMem->mSize);
	memcpy(mainMemoryBuffer, tMem->mData, tMem->mSize);

	compressMemory(tMem, &mainMemoryBuffer, tMem->mSize);

	freeTextureHW(tMem->mData);
	tMem->mData = mainMemoryBuffer;
}

void unvirtualizeTextureDreamcast(const TextureMemory& tMem)
{
	decompressMemory(tMem, &tMem->mData);
	void* textureMemoryBuffer = allocTextureHW(tMem->mSize);
	memcpy(textureMemoryBuffer, tMem->mData, tMem->mSize);
	free(tMem->mData);
	tMem->mData = textureMemoryBuffer;
}

#define virtualizeTextureHW virtualizeTextureDreamcast
#define unvirtualizeTextureHW unvirtualizeTextureDreamcast

#elif defined _WIN32 || defined __EMSCRIPTEN__ || defined(VITA)

#include <zstd.h>
#ifdef VITA
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#include <GL/glew.h>
#endif
#include "prism/texture.h"

#ifdef VITA
static void* getActiveUserData();
#endif

void* allocGLTexture(size_t) {
	GLTextureData* data = (GLTextureData*)malloc(sizeof(GLTextureData));
	return data;
}

void freeGLTexture(void* tData) {
	GLTextureData* e = (GLTextureData*)tData;
	glDeleteTextures(1, &e->mTexture);
	free(tData);
}

#define allocTextureHW allocGLTexture
#define freeTextureHW freeGLTexture

void virtualizeTextureGL(const TextureMemory&)
{
	// Unsupported
}

void unvirtualizeTextureGL(const TextureMemory&)
{
	// Unsupported
}

#define virtualizeTextureHW virtualizeTextureGL
#define unvirtualizeTextureHW unvirtualizeTextureGL

#endif

static void addToUsageQueueFront(TextureMemory tMem);
static void removeFromUsageQueue(TextureMemory tMem);
static void makeSpaceInTextureMemory(size_t tSize);

static void* allocTextureFunc(size_t tSize) {

	makeSpaceInTextureMemory(tSize);

	TextureMemory ret = (TextureMemory)malloc(sizeof(struct TextureMemory_internal));
	ret->mData = allocTextureHW(tSize);
	ret->mSize = tSize;
	ret->mIsVirtual = 0;
	ret->mIsCompressed = 0;
	addToUsageQueueFront(ret);

	return ret;
}

static void freeTextureFunc(void* tData) {
	TextureMemory tMem = (TextureMemory)tData;
	if(tMem->mIsVirtual) {
		free(tMem->mData);
	} else {
		removeFromUsageQueue(tMem);
		freeTextureHW(tMem->mData);
	}
	free(tMem);
}

#define MEMORY_STACK_MAX 10

typedef struct {
	set<void*> mMap;
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
	
#ifdef VITA
	void* mActiveUserData;
#endif

	int mActive;
} gMemoryHandler;

#ifdef VITA
static void setActiveUserData(void* tUserData)
{
	gMemoryHandler.mActiveUserData = tUserData;
}

static void* getActiveUserData()
{
	return gMemoryHandler.mActiveUserData;
}
#endif

static void initHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap) {
	(void)tStrategy;
	tMap->mMap.clear();
}

static void* addAllocatedMemoryToMemoryHandlerMapHashMapStrategy(MemoryHandlerMap* tMap, void* tData) {
	tMap->mMap.insert(tData);
	
	return tData;
}

static void* addMemoryToMemoryHandlerMapHashMapStrategy(AllocationStrategy* tStrategy, MemoryHandlerMap* tMap, int tSize) {
	void* data = tStrategy->mMalloc(tSize);
	gMemoryHandler.mAllocatedMemory++;

	return addAllocatedMemoryToMemoryHandlerMapHashMapStrategy(tMap, data);
}

static int removeMemoryFromMemoryHandlerMapWithoutFreeingMemoryHashMapStrategy(MemoryHandlerMap* tMap, void* tData) {
	set<void*>::iterator it = tMap->mMap.find(tData);
	if (it == tMap->mMap.end()) return 0;

	tMap->mMap.erase(it);

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
	typename set<void*>::iterator it = tMap->mMap.begin();

	while (it != tMap->mMap.end()) {
		void* data = *it;
		it++;
		gMemoryHandler.mAllocatedMemory--;
		tStrategy->mFreeFunc(data);
	}
	tMap->mMap.clear();
}

static AllocationStrategy getHashMapStrategyMainMemory() {
	AllocationStrategy ret;
	ret.mInitMemoryHandlerMap = initHashMapStrategy;
	ret.mAddMemoryToMemoryHandlerMap = addMemoryToMemoryHandlerMapHashMapStrategy;
	ret.mRemoveMemoryFromMemoryHandlerMap = removeMemoryFromMemoryHandlerMapHashMapStrategy;
	ret.mResizeMemoryOnMemoryHandlerMap = resizeMemoryOnMemoryHandlerMapHashMapStrategy;
	ret.mEmptyMemoryHandlerMap = emptyMemoryHandlerMapHashMapStrategy;

	ret.mMalloc = malloc;
	ret.mFreeFunc = free;
	ret.mReallocFunc = realloc;

	return ret;
}

static AllocationStrategy getHashMapStrategyTextureMemory() {
	AllocationStrategy ret;
	ret.mInitMemoryHandlerMap = initHashMapStrategy;
	ret.mAddMemoryToMemoryHandlerMap = addMemoryToMemoryHandlerMapHashMapStrategy;
	ret.mRemoveMemoryFromMemoryHandlerMap = removeMemoryFromMemoryHandlerMapHashMapStrategy;
	ret.mResizeMemoryOnMemoryHandlerMap = resizeMemoryOnMemoryHandlerMapHashMapStrategy;
	ret.mEmptyMemoryHandlerMap = emptyMemoryHandlerMapHashMapStrategy;

	ret.mMalloc = allocTextureFunc;
	ret.mFreeFunc = freeTextureFunc;
	ret.mReallocFunc = NULL;
	return ret;
}

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

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4505)
#endif

static const int COMPRESSION_BUFFER = 400;

static void compressMemory(TextureMemory tMem, void** tBuffer, int tSrcSize) {
	if (!gMemoryHandler.mIsCompressionActive) {
		tMem->mIsCompressed = 0;
		return;
	}

	char* src = (char*)(*tBuffer);
	int dstBufferSize = tSrcSize + COMPRESSION_BUFFER;
	char* dst = (char*)malloc(dstBufferSize);
	auto dstLength = ZSTD_compress(dst, size_t(dstBufferSize), src, size_t(tSrcSize), 1);
	dst = (char*)realloc(dst, dstLength);

	free(src);
	*tBuffer = dst;
	tMem->mIsCompressed = 1;
	tMem->mCompressedSize = dstLength;
}

static void decompressMemory(TextureMemory tMem, void** tBuffer) {
	if (!tMem->mIsCompressed) {
		return;
	}
	
	char* src = (char*)(*tBuffer);
	size_t uncompressedLength = (size_t)ZSTD_getFrameContentSize(src, tMem->mCompressedSize);

	char* dst = (char*)malloc(uncompressedLength);
	auto dstLength = ZSTD_decompress(dst, uncompressedLength, src, tMem->mCompressedSize);
	dst = (char*)realloc(dst, dstLength);

	free(src);
	*tBuffer = dst;	
}

#ifdef _WIN32
#pragma warning( pop ) 
#endif

static void virtualizeSingleTextureMemory(TextureMemory tMem) {
	debugLog("Virtualizing texture memory.");
	debugInteger(tMem->mSize);

	virtualizeTextureHW(tMem);

	tMem->mIsVirtual = 1;
}

static void unvirtualizeSingleTextureMemory(TextureMemory tMem) {
	debugLog("Unvirtualizing texture memory.");
	debugInteger(tMem->mSize);
  
	unvirtualizeTextureHW(tMem);

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
		sizeLeft -= int(cur->mSize);
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
	int available = getAvailableTextureMemory();

	if ((int)tSize <= available) return;

	int needed = int(tSize) - available;

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

#ifdef VITA
TextureMemory allocTextureMemory(int tSize, void* userData) {
#else
TextureMemory allocTextureMemory(int tSize) {
#endif
	if (!tSize) {
		return NULL;
	}
	
#ifdef VITA
	setActiveUserData(userData);
#endif
	return (TextureMemory)addMemoryToMemoryListStack(&gMemoryHandler.mTextureMemoryStack, tSize);
}

void freeTextureMemory(TextureMemory tMem) {
	if (tMem == NULL) return;

	removeMemoryFromMemoryListStack(&gMemoryHandler.mTextureMemoryStack, tMem);
}

void referenceTextureMemory(TextureMemory tMem) {
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
#ifdef VITA
	gMemoryHandler.mActiveUserData = NULL;
#endif
	initMemoryListStack(&gMemoryHandler.mMemoryStack, getHashMapStrategyMainMemory());
	initMemoryListStack(&gMemoryHandler.mTextureMemoryStack, getHashMapStrategyTextureMemory());
	initTextureMemoryUsageList();
	pushMemoryStack();
	pushTextureMemoryStack();
}

void shutdownMemoryHandler() {
	if (!gMemoryHandler.mActive) return;

	gMemoryHandler.mActive = 0;
	emptyMemoryListStack(&gMemoryHandler.mMemoryStack);
	emptyMemoryListStack(&gMemoryHandler.mTextureMemoryStack);
}

void setMemoryHandlingInactive()
{
	gMemoryHandler.mActive = 0;
}


void setMemoryHandlerCompressionActive() {
	gMemoryHandler.mIsCompressionActive = 1;
}

void setMemoryHandlerCompressionInactive() {
	gMemoryHandler.mIsCompressionActive = 0;
}
