#include "include/log.h"

#include <kos.h>

#include <malloc.h>

void logTextureMemoryState(){
	pvr_mem_stats();
}

void logMemoryState(){
	malloc_stats();
}
