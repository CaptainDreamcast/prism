#include "prism/logoscreen.h"

#include <stdio.h>

#include <prism/animation.h>
#include <prism/screeneffect.h>
#include <prism/timer.h>
#include <prism/input.h>
#include <prism/file.h>

static struct {
	Screen* mNextScreen;

	TextureData mBGTexture;
	AnimationHandlerElement* mBG;

	int mHasSetFadeOutColor;
	Color mFadeOutColor;
} gPrismLogoScreenData;

extern char gLogoScreenFileName[100];

static void leaveLogoScreen() {
	if (!gPrismLogoScreenData.mNextScreen) abortScreenHandling();
	else setNewScreen(gPrismLogoScreenData.mNextScreen);
}

static void endLogoFadeOut(void* tCaller) {
	(void)tCaller;
	leaveLogoScreen();
}

static void startLogoFadeOut(void* tCaller) {
	(void)tCaller;

	if (gPrismLogoScreenData.mHasSetFadeOutColor) {
		setFadeColor(gPrismLogoScreenData.mFadeOutColor);
	}

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
	gPrismLogoScreenData.mBGTexture = loadTexture(bgPath);
	gPrismLogoScreenData.mBG = playOneFrameAnimationLoop(makePosition(0,0,1), &gPrismLogoScreenData.mBGTexture);

	addFadeIn(20, logoFadeInOver, NULL);
}

static Screen* getNextLogoScreenScreen() {

	if (hasPressedAbortFlank()) {
		abortScreenHandling(); 
	}

	if (hasPressedStartFlank()) {
		return gPrismLogoScreenData.mNextScreen;
	}

	return NULL;
}

static Screen gLogoScreen;

Screen* getLogoScreenFromWrapper() {
	gLogoScreen = makeScreen(loadWrapperLogoScreen, NULL, NULL, NULL, getNextLogoScreenScreen);
	return &gLogoScreen;
}

void setScreenAfterWrapperLogoScreen(Screen * tScreen)
{
	gPrismLogoScreenData.mNextScreen = tScreen;
}

void setLogoScreenFadeOutColor(Color tColor) {
	gPrismLogoScreenData.mFadeOutColor = tColor;
	gPrismLogoScreenData.mHasSetFadeOutColor = 1;
}
