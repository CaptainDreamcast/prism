#include "prism/profiling.h"

#ifdef PRISM_PROFILING_ACTIVE
#ifdef NDEBUG
#pragma comment (lib, "OptickCore.lib")
#else
#pragma comment (lib, "OptickCored.lib")
#endif

void initProfiling() {
	// Setting memory allocators
	OPTICK_SET_MEMORY_ALLOCATOR(
		[](size_t size) -> void* { return operator new(size); },
		[](void* p) { operator delete(p); },
		[]() { /* Do some TLS initialization here if needed */ }
	);
}

void shutdownProfiling() {
	OPTICK_SHUTDOWN();
}
#endif