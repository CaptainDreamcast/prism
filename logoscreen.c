#include "tari/logoscreen.h"

#include <stdio.h>

#include <tari/animation.h>
#include <tari/screeneffect.h>
#include <tari/timer.h>
#include <tari/input.h>

static struct {
	Screen* mNextScreen;

	TextureData mBGTexture;
	int mBG;
} gData;

extern char gLogoScreenFileName[100];

static void leaveLogoScreen() {
	if (!gData.mNextScreen) abortScreenHandling();
	else setNewScreen(gData.mNextScreen);
}

static void endLogoFadeOut(void* tCaller) {
	(void)tCaller;
	leaveLogoScreen();
}

static void startLogoFadeOut(void* tCaller) {
	(void)tCaller;

	addFadeOut(20, endLogoFadeOut, NULL);
}

static void logoFadeInOver(void* tCaller) {
	(void)tCaller;

	addTimerCB(200, startLogoFadeOut, NULL);
}

static void loadWrapperLogoScreen() {
	char bgPath[1024];
	sprintf(bgPath, "logo/%s.pkg", gLogoScreenFileName);

	if (!canLoadTexture(bgPath)) {
		leaveLogoScreen();
		return;
	}
	gData.mBGTexture = loadTexture(bgPath);
	gData.mBG = playOneFrameAnimationLoop(makePosition(0,0,1), &gData.mBGTexture);

	addFadeIn(20, logoFadeInOver, NULL);
}

static Screen* getNextLogoScreenScreen() {

	if (hasPressedAbortFlank()) {
		abortScreenHandling(); 
	}

	if (hasPressedStartFlank()) {
		return gData.mNextScreen;
	}

	return NULL;
}

static Screen LogoScreenFromWrapper = {
	.mLoad = loadWrapperLogoScreen,
	.mGetNextScreen = getNextLogoScreenScreen,
};

Screen* getLogoScreenFromWrapper() {
	return &LogoScreenFromWrapper;
}

void setScreenAfterWrapperLogoScreen(Screen * tScreen)
{
	gData.mNextScreen = tScreen;
}