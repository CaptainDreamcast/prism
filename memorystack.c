#include "prism/memorystack.h"

#include <stdio.h>

#include <prism/memoryhandler.h>

MemoryStack createMemoryStack(uint32_t tSize)
{
	MemoryStack ret;
	ret.mAddress = allocMemory(tSize);
	ret.mSize = tSize;
	ret.mOffset = 0;
	ret.mAmount = 0;
	return ret;
}

void destroyMemoryStack(MemoryStack * tStack)
{
	freeMemory(tStack->mAddress);
}

void * allocMemoryOnMemoryStack(MemoryStack * tStack, uint32_t tSize)
{
	void* ret = (void*)((uint32_t)tStack->mAddress + tStack->mOffset);
	tStack->mOffset += tSize;
	//printf("%d alloc: %d\n", tStack->mAmount, tSize);
	tStack->mAmount++;
	return ret;
}
