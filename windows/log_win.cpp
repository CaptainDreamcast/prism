#include "prism/log.h"

#include <malloc.h>

#include <prism/debug.h>

void logTextureMemoryState(){
	// Not applicable / desirable / important under Windows
}

#ifdef _WIN32
#define Rectangle Rectangle2
#include <Windows.h>
#undef Rectangle
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

void hardwareLogToFile(FileHandler& tFileHandler, const char* tText) {
	if (!isInDevelopMode()) return;
	if (isOnWeb()) return;

	auto prevLogType = getMinimumLogType();
	setMinimumLogType(LOG_TYPE_NONE);

	if (tFileHandler == FILEHND_INVALID) {
		createDirectory("$pc/debug");
		tFileHandler = fileOpen("$pc/debug/log.txt", O_WRONLY);
	}
	if (tFileHandler == FILEHND_INVALID) return;

	fileWrite(tFileHandler, tText, strlen(tText));
	fileFlush(tFileHandler);

	setMinimumLogType(prevLogType);
}