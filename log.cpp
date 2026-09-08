#include "prism/log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <math.h>

#include "prism/math.h"
#include "prism/file.h"
#include "prism/debug.h"
#include "prism/system.h"

namespace prism {

#define MAX_LOG_AMOUNT 10
#define MAX_LOG_ENTRY_AMOUNT (MAX_LOG_AMOUNT + 1)

	static struct {
		LogType mMinimumLogType;

		int mPointer;
		int mAmount;
		LogEntry mLog[MAX_LOG_ENTRY_AMOUNT];

		FileHandler mLogFile = FILEHND_INVALID;

		LogCallback mCallback = NULL;
		void* mCallbackCaller = NULL;
	} gPrismLogData;

	void logprintf(const char* tFormatString, ...) {
		char* logEntry = gPrismLogData.mLog[gPrismLogData.mPointer].mText;
		char* writePoint = gPrismLogData.mLog[gPrismLogData.mPointer].mText + strlen(logEntry);
		va_list args;
		va_start(args, tFormatString);
		vsprintf(writePoint, tFormatString, args);
		va_end(args);
	}

	static void notifyLogCallback(const LogEntry& tEntry) {
		if (!gPrismLogData.mCallback) return;
		gPrismLogData.mCallback(gPrismLogData.mCallbackCaller, tEntry);
	}

	static void printLogEntry(const LogEntry& tEntry) {
		printLogColorStart(tEntry.mType);
		printf("%s", tEntry.mText);
		printLogColorEnd(tEntry.mType);
		hardwareLogToFile(gPrismLogData.mLogFile, tEntry.mText);
	}

	void logCommit(LogType tType) {
		LogEntry& entry = gPrismLogData.mLog[gPrismLogData.mPointer];
		entry.mType = tType;

		notifyLogCallback(entry);
		if (tType >= gPrismLogData.mMinimumLogType) {
			printLogEntry(entry);
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

	LogType getMinimumLogType()
	{
		return gPrismLogData.mMinimumLogType;
	}

	void setMinimumLogType(LogType tType) {
		gPrismLogData.mMinimumLogType = tType;
	}

	void setLogCallback(LogCallback tCallback, void* tCaller)
	{
		gPrismLogData.mCallback = tCallback;
		gPrismLogData.mCallbackCaller = tCaller;
	}

	void resetLogCallback()
	{
		setLogCallback(NULL, NULL);
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

}