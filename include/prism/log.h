#pragma once

#include <stdio.h>

#include "datastructures.h"


typedef enum {
	LOG_TYPE_NORMAL,
	LOG_TYPE_WARNING,
	LOG_TYPE_ERROR,
	LOG_TYPE_NONE

} LogType;

typedef struct {
	char mText[1024];
	LogType mType;
} LogEntry;

void logprintf(const char* tFormatString, ...);
void logCommit(LogType tType);
#define logBegin() {logprintf("[%s::%s, line %d] ", __FILE__, __FUNCTION__, __LINE__);}	
#define logGeneral(type, x)	{logBegin(); logprintf(x); logprintf("\n"); logCommit(type);}
#define logIntegerGeneral(type, x) {logBegin(); logprintf("Value of %s: %d\n", #x, (int)x); logCommit(type);}
#define logStringGeneral(type, x) {logBegin(); logprintf("Value of %s: %s\n", #x, x); logCommit(type);}
#define logWStringGeneral(type, x) {logBegin(); logprintf("Value of %s: %S\n", #x, x); logCommit(type);} 
#define logDoubleGeneral(type, x) {logBegin(); logprintf("Value of %s: %f\n", #x, (double)x); logCommit(type);}
#define logHexGeneral(type, x) {logBegin(); logprintf("Value of %s: %X\n", #x, (unsigned int)x); logCommit(type);}
#define logPointerGeneral(type, x) {logBegin(); logprintf("Value of %s: %p\n", #x, (char*)x); logCommit(type);}
#define logFormatGeneral(type, x, ...) {logBegin(); logFormatFunc(x,  __VA_ARGS__); logCommit(type);}
void logFormatFunc(const char* tFormatString, ...);

#define logg(x)	logGeneral(LOG_TYPE_NORMAL, x)
#define logInteger(x) logIntegerGeneral(LOG_TYPE_NORMAL, x)
#define logString(x) logStringGeneral(LOG_TYPE_NORMAL, x)
#define logWString(x) logWStringGeneral(LOG_TYPE_NORMAL, x)
#define logDouble(x) logDoubleGeneral(LOG_TYPE_NORMAL, x)
#define logHex(x) logHexGeneral(LOG_TYPE_NORMAL, x)
#define logPointer(x) logPointerGeneral(LOG_TYPE_NORMAL, x)
#define logFormat(x, ...) logFormatGeneral(LOG_TYPE_NORMAL, x,  __VA_ARGS__)

#define logError(x) logGeneral(LOG_TYPE_ERROR, x)
#define logErrorInteger(x) logIntegerGeneral(LOG_TYPE_ERROR, x)
#define logErrorString(x) logStringGeneral(LOG_TYPE_ERROR, x)
#define logErrorDouble(x) logDoubleGeneral(LOG_TYPE_ERROR, x)
#define logErrorHex(x) logHexGeneral(LOG_TYPE_ERROR, x)
#define logErrorPointer(x) logPointerGeneral(LOG_TYPE_ERROR, x)
#define logErrorFormat(x, ...) logFormatGeneral(LOG_TYPE_ERROR, x,  __VA_ARGS__)

#ifdef LOGGER_WARNINGS_DISABLED
#define logWarning(x) {}
#define logWarningInteger(x) {}
#define logWarningString(x) {}
#define logWarningFormat(x, ...) {}
#else 
#define logWarning(x) logGeneral(LOG_TYPE_WARNING, x)
#define logWarningInteger(x) logIntegerGeneral(LOG_TYPE_WARNING, x)
#define logWarningString(x) logStringGeneral(LOG_TYPE_WARNING, x)
#define logWarningFormat(x, ...) logFormatGeneral(LOG_TYPE_WARNING, x,  __VA_ARGS__)
#endif

#ifdef DEBUG
#define debugLog(x) logg(x)
#define debugInteger(x) logInteger(x)
#define debugDouble(x) logDouble(x)
#define debugString(x) logString(x)
#define debugPointer(x) logPointer(x)
#define debugFormat(x, ...) logFormatGeneral(LOG_TYPE_NORMAL, x,  __VA_ARGS__)
#else
#define debugLog(x) {}
#define debugInteger(x) {}
#define debugDouble(x) {}
#define debugString(x) {}
#define debugPointer(x) {}
#define debugFormat(x, ...) {}
#endif

#ifdef PRISM_LOG_VERBOSE
#define verboseLog(x) logg(x)
#define verboseInteger(x) logInteger(x)
#define verboseDouble(x) logDouble(x)
#define verboseString(x) logString(x)
#define verbosePointer(x) logPointer(x)
#define verboseFormat(x, ...) logFormatGeneral(LOG_TYPE_NORMAL, x,  __VA_ARGS__)
#else
#define verboseLog(x) {}
#define verboseInteger(x) {}
#define verboseDouble(x) {}
#define verboseString(x) {}
#define verbosePointer(x) {}
#define verboseFormat(x, ...) {}
#endif

void logTextureMemoryState();
void logMemoryState();

#ifdef DEBUG
#define debugLogTextureMemoryState() logTextureMemoryState()
#define debugLogMemoryState() logMemoryState()
#else
#define debugLogTextureMemoryState() {}
#define debugLogMemoryState() {}
#endif

void setMinimumLogType(LogType tType);
Vector getLogEntries(); // contains LogEntry

void printLogColorStart(LogType tType);
void printLogColorEnd(LogType tType);