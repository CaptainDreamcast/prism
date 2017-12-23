#ifndef TARI_MEMORYHANDLER_H
#define TARI_MEMORYHANDLER_H

#include <stdlib.h>

#include "common/header.h"

#ifdef DREAMCAST
#include <kos.h>

typedef pvr_ptr_t Texture;

#elif defined _WIN32 || defined __EMSCRIPTEN__
#include <SDL.h>

typedef struct {
	SDL_Texture* mTexture;
	SDL_Surface* mSurface;
} SDLTextureData;

typedef SDLTextureData* Texture;

#endif

struct TextureMemory_internal {
	void* mData;
	size_t mSize;

	int mIsVirtual;

	struct TextureMemory_internal* mPrevInUsageList;
	struct TextureMemory_internal* mNextInUsageList;
};

typedef struct TextureMemory_internal* TextureMemory;

fup void* allocMemory(int tSize);
fup void freeMemory(void* tData);
fup void* reallocMemory(void* tData, int tSize);
fup TextureMemory allocTextureMemory(int tSize);
fup void freeTextureMemory(TextureMemory tMem);
fup void referenceTextureMemory(TextureMemory tMem);

fup void pushMemoryStack();
fup void popMemoryStack();
fup void pushTextureMemoryStack();
fup void popTextureMemoryStack();

fup void initMemoryHandler();
fup void shutdownMemoryHandler();

void setMemoryHandlerCompressionActive();
void setMemoryHandlerCompressionInactive();

fup void debugPrintMemoryStack();
fup int getAvailableTextureMemory();

#endif
