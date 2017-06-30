#include "tari/memoryhandler.h"

#include "tari/math.h"
#include "tari/system.h"
#include "tari/log.h"

static struct {

	int mAvailableMemory;

} gData;

int getAvailableTextureMemory() {
	return gData.mAvailableMemory;
}

void initMemoryHandlerHW() {
	gData.mAvailableMemory = 3994752; 
	// ca. 4519040 is the full available memory
	// KOS does not allocate memory perfectly though
	// TODO: fix
}

void increaseAvailableTextureMemoryHW(size_t tSize) {
	gData.mAvailableMemory += tSize;
}

void decreaseAvailableTextureMemoryHW(size_t tSize) {
	if(gData.mAvailableMemory < (int)tSize) {
		logError("Completely out of texture memory. Virtualization failed.");
		logErrorInteger(gData.mAvailableMemory);
		logErrorInteger(tSize);
		abortSystem();
	}

	gData.mAvailableMemory -= tSize;
}


