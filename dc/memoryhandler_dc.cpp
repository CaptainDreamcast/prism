#include "prism/memoryhandler.h"

int getAvailableTextureMemory() {
	static const int SAFETY_BUFFER = 1000000;
	return pvr_mem_available() - SAFETY_BUFFER;
}

void logMemoryPlatform() {
	malloc_stats();
}