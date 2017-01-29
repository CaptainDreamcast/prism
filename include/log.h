#ifndef TARI_LOG
#define TARI_LOG

#include <stdio.h>

#define logBase() {printf("[%s::%s, line %d] ", __FILE__, __FUNCTION__, __LINE__);}	
#define log(x)	{logBase(); printf(x); printf("\n");}
#define logInteger(x) {logBase(); printf("Value of %s: %d\n", #x, (int)x);}
#define logString(x) {logBase(); printf("Value of %s: %s\n", #x, x);}
#define logDouble(x) {logBase(); printf("Value of %s: %f\n", #x, (double)x);}

#define logError(x) log(x)
#define logErrorInteger(x) logInteger(x)
#define logErrorString(x) logString(x)

#define logWarning(x) log(x)
#define logWarningInteger(x) logInteger(x)
#define logWarningString(x) logString(x)

#ifdef DEBUG
#define debugLog(x) log(x)
#define debugInteger(x) logInteger(x)
#define debugDouble(x) logDouble(x)
#define debugString(x) logString(x)
#else
#define debugLog(x) {}
#define debugInteger(x) {}
#define debugDouble(x) {}
#define debugString(x) {}
#endif	

#ifdef DEBUG
void logTextureMemoryState();
void logMemoryState();
#else
#define logTextureMemoryState() {}
#define logMemoryState() {}
#endif

#endif
