#pragma once

#include <stdlib.h>
#include <string_view>

#ifdef DREAMCAST
#include <kos.h>

namespace prism {
typedef pvr_ptr_t Texture;

#elif defined _WIN32 || defined __EMSCRIPTEN__
#include <SDL.h>
#include <GL/glew.h>

namespace prism {

typedef struct {
	GLuint mTexture;
} GLTextureData;

typedef GLTextureData* Texture;

#elif defined(VITA)

#include <vita2d.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/kernel/sysmem.h>

namespace prism {

	typedef struct {
		vita2d_texture* mTexture;
} VitaTextureData;

typedef VitaTextureData* Texture;

void* vitaGpuAlloc(SceKernelMemBlockType type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID* uid);
void vitaGpuFree(SceUID uid);

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
int getAvailableSoundMemory();

int getAllocatedMemoryBlockAmount();

#ifdef _WIN32
void imguiMemoryHandler();
void imguiTextureMemory(const std::string_view& tName, const TextureMemory& tTextureMemory);
#endif

}