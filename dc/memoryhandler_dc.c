#include "prism/memoryhandler.h"

#include "prism/math.h"
#include "prism/system.h"
#include "prism/log.h"

static struct {

	int mAvailableMemory;

} gData;

int getAvailableTextureMemory() {
	return gData.mAvailableMemory;
}

void initMemoryHandlerHW() {
	gData.mAvailableMemory = 5294752; 
	// ca. 4519040 is the full available memory for 480p
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


