#include "prism/memoryhandler.h"

#include <dc/sound/sound.h>

int getAvailableTextureMemory() {
	static const int SAFETY_BUFFER = 1000000;
	return pvr_mem_available() - SAFETY_BUFFER;
}

int getAvailableSoundMemory() {
	return snd_mem_available();
}