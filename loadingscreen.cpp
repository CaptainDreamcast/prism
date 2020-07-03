#include "prism/loadingscreen.h"

#include "prism/thread.h"
#include "prism/drawing.h"
#include "prism/mugentexthandler.h"
#include "prism/system.h"
#include "prism/screeneffect.h"

#ifdef DREAMCAST
static struct {
	int mTicks;
} gPrismLoadingScreenData;

extern semaphore_t gPVRAccessSemaphore;

static void drawLoadingText() {

	char text[100];
	strcpy(text, "Loading");
	int pos = strlen(text);
	int i;
	for(i = 0; i < gPrismLoadingScreenData.mTicks; i++) text[pos++] = '.';
	text[pos] = '\0';
	drawMugenText(text, Vector3D(20, 20, 10), -1);
}

static void loadScreenLoop() {


	waitForScreen();

	sem_wait(&gPVRAccessSemaphore);
	startDrawing();
	drawLoadingText();
	stopDrawing();
  	sem_signal(&gPVRAccessSemaphore);
	//printf("liv %d\n", gPrismLoadingScreenData.mTicks);
	//thd_sleep(1000);

	gPrismLoadingScreenData.mTicks = (gPrismLoadingScreenData.mTicks+1) % 10;
}


void startLoadingScreen(int* tHasFinishedLoadingReference)
{
	setScreenBackgroundColorRGB(0, 0, 0);
	gPrismLoadingScreenData.mTicks = 0;
	while(!(*tHasFinishedLoadingReference)) {
		loadScreenLoop();
	}
}
#else

void startLoadingScreen(int* /*tHasFinishedLoadingReference*/)
{
	
}

#endif

