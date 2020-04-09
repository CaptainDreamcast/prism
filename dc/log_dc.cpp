#include <kos.h>

#include <malloc.h>

#include "prism/debug.h"

void logTextureMemoryState(){
	if (!isInDevelopMode()) return;
	pvr_mem_stats();
}

void logMemoryState(){
	if (!isInDevelopMode()) return;
	malloc_stats();
}

void printLogColorStart(LogType tType) {
	switch (tType) {
	case LOG_TYPE_WARNING:
		printf("\033[0;33m");
		break;
	case LOG_TYPE_ERROR:
		printf("\033[1;31m");
		break;
	default:
		break;
	}
}

void printLogColorEnd(LogType tType) {
	switch (tType) {
	case LOG_TYPE_WARNING:
	case LOG_TYPE_ERROR:
		printf("\033[0m");
		fflush(stdout);
		break;
	default:
		break;
	}
}