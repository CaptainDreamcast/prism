#include "prism/loadingscreen.h"

#include "prism/thread.h"
#include "prism/drawing.h"
#include "prism/mugentexthandler.h"
#include "prism/system.h"

static struct {
	int mThreadID;
	int mIsActive;
	Semaphore mRunning;
} gData;

static void loadScreenLoop() {

	waitForScreen();
	startDrawing();
	drawMugenText("Loading...", makePosition(20, 20, 10), 1);
	stopDrawing();
}

static void loadScreenThread(void* tCaller) {
	(void)tCaller;
	
	while (gData.mIsActive) {
		loadScreenLoop();
	}

	releaseSemaphore(gData.mRunning);
}

void startLoadingScreen()
{
	if (!isOnDreamcast()) return; // TODO
	gData.mIsActive = 1;
	gData.mRunning = createSemaphore(0);
	gData.mThreadID = startThread(loadScreenThread, NULL);
}

void endLoadingScreen()
{
	if (!isOnDreamcast()) return; // TODO

	gData.mIsActive = 0;
	lockSemaphore(gData.mRunning);
}
