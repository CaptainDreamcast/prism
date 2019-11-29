#pragma once

#include <stdint.h>

typedef struct {
	uint32_t mSize;
	uint32_t mOffset;
	void* mAddress;
	int mAmount;
} MemoryStack;

MemoryStack createMemoryStack(uint32_t tSize);
void destroyMemoryStack(MemoryStack* tStack);
void resizeMemoryStackToCurrentSize(MemoryStack* tStack);

void* allocMemoryOnMemoryStack(MemoryStack* tStack, uint32_t tSize);
int canFitOnMemoryStack(MemoryStack* tStack, uint32_t tSize);