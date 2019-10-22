#pragma once

#include <stdlib.h>

#ifdef DREAMCAST
#include <kos.h>

typedef pvr_ptr_t Texture;

#elif defined _WIN32 || defined __EMSCRIPTEN__
#include <SDL.h>
#include <GL/glew.h>

typedef struct {
	GLuint mTexture;
} SDLTextureData;

typedef SDLTextureData* Texture;

#endif

struct TextureMemory_internal {
	void* mData;
	size_t mSize;
	size_t mCompressedSize;

	int mIsVirtual;
	int mIsCompressed;

	struct TextureMemory_internal* mPrevInUsageList;
	struct TextureMemory_internal* mNextInUsageList;
};

typedef struct TextureMemory_internal* TextureMemory;

void* allocMemory(int tSize);
void* allocClearedMemory(int tBlockAmount, int tBlockSize);
void freeMemory(void* tData);
void* reallocMemory(void* tData, int tSize);
TextureMemory allocTextureMemory(int tSize);
void freeTextureMemory(TextureMemory tMem);
void referenceTextureMemory(TextureMemory tMem);

void pushMemoryStack();
void popMemoryStack();
void pushTextureMemoryStack();
void popTextureMemoryStack();

void initMemoryHandler();
void shutdownMemoryHandler();
void setMemoryHandlingInactive();

void setMemoryHandlerCompressionActive();
void setMemoryHandlerCompressionInactive();

void debugPrintMemoryStack();
int getAvailableTextureMemory();
void logMemoryPlatform();

int getAllocatedMemoryBlockAmount();