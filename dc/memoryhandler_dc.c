#include "tari/memoryhandler.h"

#include "tari/math.h"

int getAvailableTextureMemory() {
	return pvr_mem_available();
}
