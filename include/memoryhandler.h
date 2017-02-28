#ifndef TARI_MEMORYHANDLER_H
#define TARI_MEMORYHANDLER_H

void* allocMemory(int tSize);
void freeMemory(void* tData);
void* reallocMemory(void* tData, int tSize);
void* allocTextureMemory(int tSize);
void freeTextureMemory(void* tData);

void pushMemoryStack();
void popMemoryStack();
void pushTextureMemoryStack();
void popTextureMemoryStack();

void initMemoryHandler();
void shutdownMemoryHandler();

void debugPrintMemoryStack();

#endif
