#include "prism/system.h"

#include <time.h>
#include <stdlib.h>

#include "prism/log.h"

namespace prism {

	static struct {
		ScreenSize mScreenSize = { 640, 480 };
		int mIsLoaded = 0;
	} gSystemLinuxData;

	void initSystem() { gSystemLinuxData.mIsLoaded = 1; }
	void shutdownSystem() { gSystemLinuxData.mIsLoaded = 0; }
	void updateSystem() {}

	void abortSystem() {
		logError("Aborting system.");
		exit(1);
	}

	void updateGameName(const char* /*tName*/) {}
	void setIcon(const char* /*tPath*/) {}

	void setScreen(int tX, int tY, int /*tFramerate*/, int /*tIsVGA*/) {
		gSystemLinuxData.mScreenSize.x = tX;
		gSystemLinuxData.mScreenSize.y = tY;
	}

	void setScreenSize(int tX, int tY) {
		gSystemLinuxData.mScreenSize.x = tX;
		gSystemLinuxData.mScreenSize.y = tY;
	}

	ScreenSize getScreenSize() { return gSystemLinuxData.mScreenSize; }

	void setScreenPosition(int /*tX*/, int /*tY*/) {}
	void setScreenFullscreen(bool /*tIsFullscreen*/) {}
	ScreenSize getDisplayedScreenSize() { return gSystemLinuxData.mScreenSize; }
	void setDisplayedScreenSize(int /*tX*/, int /*tY*/) {}
	void setScreenFramerate(int /*tFramerate*/) {}
	void setVGA() {}

	int isOnDreamcast() { return 0; }
	int isOnWindows() { return 1; } /* headless build mirrors the Windows asset/logic paths */
	int isOnWeb() { return 0; }
	int isOnVita() { return 0; }

	uint64_t getSystemTicks() {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (uint64_t)ts.tv_sec * 1000 + (uint64_t)(ts.tv_nsec / 1000000);
	}

	uint64_t getUnixTimestampSeconds() { return (uint64_t)time(NULL); }

	uint64_t getUnixTimestampMilliseconds() {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return (uint64_t)ts.tv_sec * 1000 + (uint64_t)(ts.tv_nsec / 1000000);
	}

}
