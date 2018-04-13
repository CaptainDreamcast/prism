#include "prism/loadingscreen.h"

#include "prism/thread.h"
#include "prism/drawing.h"
#include "prism/mugentexthandler.h"
#include "prism/system.h"
#include "prism/screeneffect.h"

static struct {
	int mTicks;
} gData;

static void drawLoadingText() {
	char text[100];
	strcpy(text, "Loading");
	int pos = strlen(text);
	int i;
	for(i = 0; i < gData.mTicks; i++) text[pos++] = '.';
	text[pos] = '\0';
	drawMugenText(text, makePosition(20, 20, 10), 1);

	gData.mTicks = (gData.mTicks+1) % 10;
}

static void loadScreenLoop() {

	waitForScreen();
	startDrawing();
	drawLoadingText();
	stopDrawing();
}


void startLoadingScreen(int* tHasFinishedLoadingReference)
{
	gData.mTicks = 0;
	while(!(*tHasFinishedLoadingReference)) {
		loadScreenLoop();
	}
}


