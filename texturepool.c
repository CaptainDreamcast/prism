#include "tari/texturepool.h"

#include "tari/datastructures.h"
#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"

typedef struct {
	TextureData mTexture;
	char mPath[1024];
	int mCounter;
} TexturePoolEntry;

static struct {
	StringMap mPathToLoadedTexture;
	StringMap mTextureHashToLoadedTexture;

	int mIsLoaded;
} gTexturePool;

void setupTexturePool() {
	if (gTexturePool.mIsLoaded) {
		logWarning("Attempting to use active texture pool. Resetting.");
		shutdownTexturePool();
	}

	gTexturePool.mPathToLoadedTexture = new_string_map();
	gTexturePool.mTextureHashToLoadedTexture = new_string_map();

	gTexturePool.mIsLoaded = 1;
}

static void cleanSingleTexturePoolEntry(void* tCaller, char* tKey, void* tData) {
	(void) tCaller;
	(void) tKey;
	TexturePoolEntry* e = tData;
	unloadTexture(e->mTexture);
}

void shutdownTexturePool() {
	string_map_map(&gTexturePool.mTextureHashToLoadedTexture, cleanSingleTexturePoolEntry, NULL);
	delete_string_map(&gTexturePool.mTextureHashToLoadedTexture);
	delete_string_map(&gTexturePool.mPathToLoadedTexture);
	gTexturePool.mIsLoaded = 0;
}

static TextureData increaseCounterAndFetchTexture(char* tPath) {
	TexturePoolEntry* e = string_map_get(&gTexturePool.mPathToLoadedTexture, tPath);
	e->mCounter++;

	return e->mTexture;
}

TextureData loadTextureFromPool(char* tPath) {
	int isLoadNotNecessary = string_map_contains(&gTexturePool.mPathToLoadedTexture, tPath);

	if (isLoadNotNecessary) {
		return increaseCounterAndFetchTexture(tPath);
	}

	TexturePoolEntry* e = allocMemory(sizeof(TexturePoolEntry));
	e->mCounter = 1;
	e->mTexture = loadTexture(tPath);
	strcpy(e->mPath, tPath);

	char hashString[100];
	sprintf(hashString, "%d", getTextureHash(e->mTexture));
	string_map_push_owned(&gTexturePool.mPathToLoadedTexture, tPath, e);
	string_map_push(&gTexturePool.mTextureHashToLoadedTexture, hashString, e);

	return e->mTexture;
}

void unloadTextureFromPool(TextureData tTexture) {
	char hashString[100];
	sprintf(hashString, "%d", getTextureHash(tTexture));

	int hasBeenLoaded = string_map_contains(&gTexturePool.mTextureHashToLoadedTexture, hashString);
	if (!hasBeenLoaded) {
		logError("Unrecognized Texture.");
		logErrorString(hashString);
		abortSystem();
	}

	TexturePoolEntry* e = string_map_get(&gTexturePool.mTextureHashToLoadedTexture, hashString);
	e->mCounter--;

	if (e->mCounter > 0) return;

	unloadTexture(e->mTexture);
	string_map_remove(&gTexturePool.mTextureHashToLoadedTexture, hashString);
	string_map_remove(&gTexturePool.mPathToLoadedTexture, e->mPath);
}
