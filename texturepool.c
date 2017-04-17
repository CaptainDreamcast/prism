#include "include/texturepool.h"

#include "include/datastructures.h"
#include "include/log.h"
#include "include/memoryhandler.h"
#include "include/system.h"

typedef struct {
	TextureData mTexture;
	char mPath[1024];
	int mCounter;
} TexturePoolEntry;

static struct {
	StringMap mPathToLoadedTexture;
	StringMap mTextureHashToLoadedTexture;

	int mIsLoaded;
} gData;

void setupTexturePool() {
	if (gData.mIsLoaded) {
		logWarning("Attempting to use active texture pool. Resetting.");
		shutdownTexturePool();
	}
	gData.mPathToLoadedTexture = new_string_map();
	gData.mTextureHashToLoadedTexture = new_string_map();

	gData.mIsLoaded = 1;
}

static void cleanSingleTexturePoolEntry(void* tCaller, char* tKey, void* tData) {
	TexturePoolEntry* e = tData;
	unloadTexture(e->mTexture);
}

void shutdownTexturePool() {
	string_map_map(&gData.mTextureHashToLoadedTexture, cleanSingleTexturePoolEntry, NULL);
	delete_string_map(&gData.mTextureHashToLoadedTexture);
	delete_string_map(&gData.mPathToLoadedTexture);
	gData.mIsLoaded = 0;
}

static TextureData increaseCounterAndFetchTexture(char* tPath) {
	TexturePoolEntry* e = string_map_get(&gData.mPathToLoadedTexture, tPath);
	e->mCounter++;

	return e->mTexture;
}

TextureData loadTextureFromPool(char* tPath) {
	int isLoadNotNecessary = string_map_contains(&gData.mPathToLoadedTexture, tPath);

	if (isLoadNotNecessary) {
		return increaseCounterAndFetchTexture(tPath);
	}

	TexturePoolEntry* e = allocMemory(sizeof(TexturePoolEntry));
	e->mCounter = 1;
	e->mTexture = loadTexture(tPath);
	strcpy(e->mPath, tPath);

	char hashString[100];
	sprintf(hashString, "%d", getTextureHash(e->mTexture));
	string_map_push_owned(&gData.mPathToLoadedTexture, tPath, e);
	string_map_push(&gData.mTextureHashToLoadedTexture, hashString, e);

	return e->mTexture;
}

void unloadTextureFromPool(TextureData tTexture) {
	char hashString[100];
	sprintf(hashString, "%d", getTextureHash(tTexture));

	int hasBeenLoaded = string_map_contains(&gData.mTextureHashToLoadedTexture, hashString);
	if (!hasBeenLoaded) {
		logError("Unrecognized Texture.");
		logErrorString(hashString);
		abortSystem();
	}

	TexturePoolEntry* e = string_map_get(&gData.mTextureHashToLoadedTexture, hashString);
	e->mCounter--;

	if (e->mCounter > 0) return;

	unloadTexture(e->mTexture);
	string_map_remove(&gData.mTextureHashToLoadedTexture, hashString);
	string_map_remove(&gData.mPathToLoadedTexture, e->mPath);
}
