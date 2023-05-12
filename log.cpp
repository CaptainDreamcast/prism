#include "prism/log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <math.h>

#include "prism/math.h"
#include "prism/file.h"
#include "prism/debug.h"
#include "prism/system.h"

#define MAX_LOG_AMOUNT 10
#define MAX_LOG_ENTRY_AMOUNT (MAX_LOG_AMOUNT + 1)

static struct {
	LogType mMinimumLogType;

	int mPointer;
	int mAmount;
	LogEntry mLog[MAX_LOG_ENTRY_AMOUNT];

	FileHandler mLogFile = FILEHND_INVALID;
} gPrismLogData;

void logprintf(const char* tFormatString, ...) {
	char* logEntry = gPrismLogData.mLog[gPrismLogData.mPointer].mText;
	char* writePoint = gPrismLogData.mLog[gPrismLogData.mPointer].mText + strlen(logEntry);
	va_list args;
	va_start(args, tFormatString);
	vsprintf(writePoint, tFormatString, args);
	va_end(args);
}

static void logToFile() {
	if (!isInDevelopMode()) return;
	if (isOnDreamcast() || isOnWeb() || isOnVita()) return;

	if (gPrismLogData.mLogFile == FILEHND_INVALID) {
		createDirectory("$pc/debug");
		gPrismLogData.mLogFile = fileOpen("$pc/debug/log.txt", O_WRONLY);
	}
	if (gPrismLogData.mLogFile == FILEHND_INVALID) return;

	fileWrite(gPrismLogData.mLogFile, gPrismLogData.mLog[gPrismLogData.mPointer].mText, strlen(gPrismLogData.mLog[gPrismLogData.mPointer].mText));
	fileFlush(gPrismLogData.mLogFile);
}

void logCommit(LogType tType) {
	gPrismLogData.mLog[gPrismLogData.mPointer].mType = tType;

	if(tType >= gPrismLogData.mMinimumLogType) {
		printLogColorStart(tType);
		printf("%s", gPrismLogData.mLog[gPrismLogData.mPointer].mText);
		printLogColorEnd(tType);
		logToFile();
	}

	gPrismLogData.mPointer = (gPrismLogData.mPointer + 1) % MAX_LOG_ENTRY_AMOUNT;
	gPrismLogData.mAmount = gPrismLogData.mAmount + 1 > MAX_LOG_AMOUNT ? MAX_LOG_AMOUNT : gPrismLogData.mAmount + 1;
	*gPrismLogData.mLog[gPrismLogData.mPointer].mText = '\0';
}

void logFormatFunc(const char* tFormatString, ...) {
	char text[1024];
	va_list args;
	va_start(args, tFormatString);
	vsprintf(text, tFormatString, args);
	va_end(args);

	logprintf("%s\n", text);
}

void setMinimumLogType(LogType tType) {
	gPrismLogData.mMinimumLogType = tType;
}

Vector getLogEntries()
{
	Vector ret = new_vector();

	int i;
	int pointer;
	if (gPrismLogData.mAmount < MAX_LOG_AMOUNT) pointer = 0;
	else pointer = (gPrismLogData.mPointer + 1) % MAX_LOG_ENTRY_AMOUNT;
	for (i = 0; i < gPrismLogData.mAmount; i++) {
		vector_push_back(&ret, &gPrismLogData.mLog[pointer]);
		pointer = (pointer + 1) % MAX_LOG_ENTRY_AMOUNT;
	}

	return ret;
}
