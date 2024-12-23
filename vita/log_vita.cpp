#include "prism/log.h"

#include <malloc.h>

#include <prism/debug.h>

namespace prism {

	void logTextureMemoryState() {
		// UNSUPPORTED YET
	}

	void logMemoryState() {
		// UNSUPPORTED YET
	}

	void printLogColorStart(LogType /*tType*/) {}
	void printLogColorEnd(LogType /*tType*/) {}

	void hardwareLogToFile(FileHandler& tFileHandler, const char* tText) {
		if (!isInDevelopMode()) return;

		auto prevLogType = getMinimumLogType();
		setMinimumLogType(LOG_TYPE_NONE);

		if (tFileHandler == FILEHND_INVALID) {
			tFileHandler = fopen("ux0:data/prismlog.txt", "wb+");
		}
		if (tFileHandler == FILEHND_INVALID) return;

		fileWrite(tFileHandler, tText, strlen(tText));
		fileFlush(tFileHandler);

		fclose(tFileHandler); // Vita log file is super touchy about when it writes out its data and seems to ignore flush, closing it seems to help
		tFileHandler = fopen("ux0:data/prismlog.txt", "ab");

		setMinimumLogType(prevLogType);
	}

}