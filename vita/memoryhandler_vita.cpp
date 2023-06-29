#include "prism/memoryhandler.h"

#include "prism/math.h"
#include "prism/log.h"

#include <vitaGL.h>

int getAvailableTextureMemory() {
	return INF;
	//static const int SAFETY_BUFFER = 5000000;
	//return vglMemFree(VGL_MEM_VRAM) - SAFETY_BUFFER; // might make sense to include proper texture mem check in future
}

int getAvailableSoundMemory() {
	return INF;
}