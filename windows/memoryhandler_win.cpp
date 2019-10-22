#include "prism/memoryhandler.h"

#include "prism/math.h"
#include "prism/log.h"
#include "prism/debug.h"

int getAvailableTextureMemory() {
	return INF;
}

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
void logMemoryPlatform() {
	if (!isInDevelopMode()) return;
	PROCESS_MEMORY_COUNTERS memCounter;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter))) {
		logFormat("malloc_stats: Working set size: %lu", memCounter.WorkingSetSize);
	}
}
#else
void logMemoryPlatform() {
	// total memory is known at compile time or dynamic
}
#endif