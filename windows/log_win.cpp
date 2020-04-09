#include "prism/log.h"

#include <malloc.h>

#include <prism/debug.h>

void logTextureMemoryState(){
	// Not applicable / desirable / important under Windows
}

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
void logMemoryState() {
	if (!isInDevelopMode()) return;
	PROCESS_MEMORY_COUNTERS memCounter;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter))) {
		logFormat("malloc_stats: Working set size: %lu", memCounter.WorkingSetSize);
	}
}

void printLogColorStart(LogType tType) {
	HANDLE console;
	switch (tType) {
	case LOG_TYPE_WARNING:
		console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN);
		break;
	case LOG_TYPE_ERROR:
		console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_RED);
		break;
	default:
		break;
	}
}

void printLogColorEnd(LogType tType) {
	HANDLE console;
	switch (tType) {
	case LOG_TYPE_WARNING:
	case LOG_TYPE_ERROR:
		console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	default:
		break;
	}
}

#else
void logMemoryState() {
	// total memory is known at compile time or dynamic
}

// textarea used for emscripten output does not support different colors
void printLogColorStart(LogType /*tType*/) {}
void printLogColorEnd(LogType /*tType*/) {}
#endif
