#include "prism/log.h"

#include <stdio.h>
#include <stdarg.h>

#include <math.h>

#include "prism/math.h"

#define MAX_LOG_AMOUNT 10
#define MAX_LOG_ENTRY_AMOUNT (MAX_LOG_AMOUNT + 1)

static struct {
	LogType mMinimumLogType;

	int mPointer;
	int mAmount;
	LogEntry mLog[MAX_LOG_ENTRY_AMOUNT];
} gData;

void logprintf(char* tFormatString, ...) {
	char* logEntry = gData.mLog[gData.mPointer].mText;
	char* writePoint = gData.mLog[gData.mPointer].mText + strlen(logEntry);
	va_list args;
	va_start(args, tFormatString);
	vsprintf(writePoint, tFormatString, args);
	va_end(args);
}

void logCommit(LogType tType) {
	gData.mLog[gData.mPointer].mType = tType;

	if(tType >= gData.mMinimumLogType) {
		printf(gData.mLog[gData.mPointer].mText);
	}

	gData.mPointer = (gData.mPointer + 1) % MAX_LOG_ENTRY_AMOUNT;
	gData.mAmount = gData.mAmount + 1 > MAX_LOG_AMOUNT ? MAX_LOG_AMOUNT : gData.mAmount + 1; // TODO: min error
	*gData.mLog[gData.mPointer].mText = '\0';
}

void logFormatFunc(char* tFormatString, ...) {
	char text[1024];
	va_list args;
	va_start(args, tFormatString);
	vsprintf(text, tFormatString, args);
	va_end(args);

	logprintf("%s\n", text);
}

void setMinimumLogType(LogType tType) {
	gData.mMinimumLogType = tType;
}

Vector getLogEntries()
{
	Vector ret = new_vector();

	int i;
	int pointer;
	if (gData.mAmount < MAX_LOG_AMOUNT) pointer = 0;
	else pointer = (gData.mPointer + 1) % MAX_LOG_ENTRY_AMOUNT;
	for (i = 0; i < gData.mAmount; i++) {
		vector_push_back(&ret, &gData.mLog[pointer]);
		pointer = (pointer + 1) % MAX_LOG_ENTRY_AMOUNT;
	}

	return ret;
}
