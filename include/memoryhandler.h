#ifndef TARI_MEMORYHANDLER_H
#define TARI_MEMORYHANDLER_H

#include "common/header.h"

#ifdef DREAMCAST
#include <kos.h>

typedef pvr_ptr_t Texture;

#elif defined _WIN32
#include <SDL.h>

typedef struct {
	SDL_Texture* mTexture;

} SDLTextureData;

typedef SDLTextureData* Texture;

#endif

struct TextureMemory_internal {
	Texture mData;
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

fup void debugPrintMemoryStack();

#endif
