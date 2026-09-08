#include "prism/log.h"

#include <malloc.h>

#include <psp2/kernel/sysmem.h>

#include <prism/debug.h>

namespace prism {

	void logTextureMemoryState() {
		SceKernelFreeMemorySizeInfo info;
		info.size = sizeof(info);
		if (sceKernelGetFreeMemorySize(&info) < 0) return;
		logFormat("Free memory: cdram %d, phycont %d, user %d", info.size_cdram, info.size_phycont, info.size_user);
	}

	void logMemoryState() {
		const auto info = mallinfo();
		logFormat("Heap: %d used, %d free, %d reserved", (int)info.uordblks, (int)info.fordblks, (int)info.arena);
		logTextureMemoryState();
	}

	void printLogColorStart(LogType /*tType*/) {}
	void printLogColorEnd(LogType /*tType*/) {}

	void hardwareLogToFile(FileHandler& tFileHandler, const char* tText) {
		if (getMinimumLogType() == LOG_TYPE_NONE) return;

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