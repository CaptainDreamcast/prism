#include "prism/system.h"

#include <string.h>
#include <assert.h>
#include <ctime>

#include <stdlib.h>
#include <vita2d.h>
#include <psp2/rtc.h> 

#include "prism/log.h"
#include "prism/geometry.h"
#include "prism/math.h"
#include "prism/wrapper.h"
#include "prism/profiling.h"
#include "prism/input.h"

namespace prism {

	void abortSystem() {
		assert(0);
		exit(0);
	}

	static struct {

		int mIsLoaded;

		int mScreenSizeX;
		int mScreenSizeY;

		int mDisplayedWindowSizeX;
		int mDisplayedWindowSizeY;

		char mGameName[100];
	} gPrismVitaSystemData;

	static void initScreenDefault() {
		gPrismVitaSystemData.mIsLoaded = 1;
		gPrismVitaSystemData.mScreenSizeX = gPrismVitaSystemData.mDisplayedWindowSizeX = 960;
		gPrismVitaSystemData.mScreenSizeY = gPrismVitaSystemData.mDisplayedWindowSizeY = 544;
	}

	void setGameName(const char* tName) {
		strcpy(gPrismVitaSystemData.mGameName, tName);
		// UNSUPPORTED
	}

	void updateGameName(const char* tName)
	{
		strcpy(gPrismVitaSystemData.mGameName, tName);
		// UNSUPPORTED
	}

	void setIcon(const char*)
	{
		// UNSUPPORTED
	}

	static void setToProgramDirectory() {
		// UNSUPPORTED
	}

	extern void setDrawingScreenScale(double tScaleX, double tScaleY);

	void initSystem() {

		setToProgramDirectory();

		if (gPrismVitaSystemData.mGameName[0] == '\0') {
			sprintf(gPrismVitaSystemData.mGameName, "Unnamed libtari game port");
		}
	}

	void shutdownSystem() {

	}

	void updateSystem() {
	}

	void setScreen(int tX, int tY, int tFramerate, int tIsVGA) {
		(void)tIsVGA;
		(void)tFramerate;
		if (!gPrismVitaSystemData.mIsLoaded) initScreenDefault();
		gPrismVitaSystemData.mScreenSizeX = tX;
		gPrismVitaSystemData.mScreenSizeY = tY;
	}

	void setScreenSize(int tX, int tY) {
		if (!gPrismVitaSystemData.mIsLoaded) initScreenDefault();

		gPrismVitaSystemData.mScreenSizeX = tX;
		gPrismVitaSystemData.mScreenSizeY = tY;
	}

	ScreenSize getScreenSize() {
		if (!gPrismVitaSystemData.mIsLoaded) initScreenDefault();
		ScreenSize ret;
		ret.x = gPrismVitaSystemData.mScreenSizeX;
		ret.y = gPrismVitaSystemData.mScreenSizeY;

		return ret;
	}

	ScreenSize getDisplayedScreenSize()
	{
		if (!gPrismVitaSystemData.mIsLoaded) initScreenDefault();
		ScreenSize ret;
		ret.x = gPrismVitaSystemData.mDisplayedWindowSizeX;
		ret.y = gPrismVitaSystemData.mDisplayedWindowSizeY;
		return ret;
	}

	void setDisplayedScreenSize(int tX, int tY)
	{
		// impossible to change under Vita
	}

	void setScreenFramerate(int tFramerate) {
		(void)tFramerate;
	}

	void setScreenPosition(int, int) {
		// impossible to change under Vita
	}
	void setScreenFullscreen(bool) {
		// impossible to change under Vita
	}

	void setVGA() {

	}

	void returnToMenu() {
		exit(0);
	}

	int isOnDreamcast()
	{
		return 0;
	}

	int isOnWindows()
	{
		return 0;
	}

	int isOnWeb()
	{
		return 0;
	}

	int isOnVita()
	{
		return 1;
	}

	uint64_t getSystemTicks() {
		SceRtcTick tick;
		sceRtcGetCurrentTick(&tick);
		return tick.tick;
	}

	uint64_t getUnixTimestampSeconds()
	{
		return std::time(0);
	}

}