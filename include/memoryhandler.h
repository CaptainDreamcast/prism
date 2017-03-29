#ifndef TARI_MEMORYHANDLER_H
#define TARI_MEMORYHANDLER_H

#include "common/header.h"

fup void* allocMemory(int tSize);
fup void freeMemory(void* tData);
fup void* reallocMemory(void* tData, int tSize);
fup void* allocTextureMemory(int tSize);
fup void freeTextureMemory(void* tData);

fup void pushMemoryStack();
fup void popMemoryStack();
fup void pushTextureMemoryStack();
fup void popTextureMemoryStack();

fup void initMemoryHandler();
fup void shutdownMemoryHandler();

fup void debugPrintMemoryStack();

#endif
