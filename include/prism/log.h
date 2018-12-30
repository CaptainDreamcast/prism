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

void logprintf(char* tFormatString, ...);
void logCommit(LogType tType);
#define logBegin() {logprintf("[%s::%s, line %d] ", __FILE__, __FUNCTION__, __LINE__);}	
#define logGeneral(type, x)	{logBegin(); logprintf(x); logprintf("\n"); logCommit(type);}
#define logIntegerGeneral(type, x) {logBegin(); logprintf("Value of %s: %d\n", #x, (int)x); logCommit(type);}
#define logStringGeneral(type, x) {logBegin(); logprintf("Value of %s: %s\n", #x, x); logCommit(type);}
#define logWStringGeneral(type, x) {logBegin(); logprintf("Value of %s: %S\n", #x, x); logCommit(type);} 
#define logDoubleGeneral(type, x) {logBegin(); logprintf("Value of %s: %f\n", #x, (double)x); logCommit(type);}
#define logHexGeneral(type, x) {logBegin(); logprintf("Value of %s: %X\n", #x, (unsigned int)x); logCommit(type);}
#define logPointerGeneral(type, x) {logBegin(); logprintf("Value of %s: %p\n", #x, (char*)x); logCommit(type);}
#define logFormatGeneral(type, x, ...) {logBegin(); logFormatFunc(x,  __VA_ARGS__); logprintf("\n"); logCommit(type);}
void logFormatFunc(char* tFormatString, ...);

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

#else
#define debugLog(x) {}
#define debugInteger(x) {}
#define debugDouble(x) {}
#define debugString(x) {}
#define debugPointer(x) {}
#endif	

#ifdef DEBUG
void logTextureMemoryState();
void logMemoryState();
#else
#define logTextureMemoryState() {}
#define logMemoryState() {}
#endif

void setMinimumLogType(LogType tType);
Vector getLogEntries(); // contains LogEntry
