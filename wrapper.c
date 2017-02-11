#include "include/wrapper.h"

#include "include/pvr.h"
#include "include/physics.h"
#include "include/file.h"
#include "include/drawing.h"
#include "include/log.h"
#include "include/memoryhandler.h"


#include "include/timer.h"
#include "include/animation.h"
#include "include/input.h"
#include "include/physicshandler.h"
#include "include/stagehandler.h"
#include "include/collisionhandler.h"

void initTariWrapperWithDefaultFlags() {
	initiatePVR();
	initMemoryHandler();
	initPhysics();
	initFileSystem();
	initDrawing();
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
}
void shutdownTariWrapper() {
	shutdownMemoryHandler();
}

static struct {
	int mIsAborted;
} gData;

static void loadScreen(Screen* tScreen) {
	pushMemoryStack();
	pushTextureMemoryStack();
	setupTimer();
	setupAnimationHandler();
	setupPhysicsHandler();
	setupStageHandler();
	setupCollisionHandler();

	tScreen->mLoad();
}

static void unloadScreen(Screen* tScreen) {
	tScreen->mUnload();

	shutdownCollisionHandler();
	shutdownStageHandler();
	shutdownPhysicsHandler();
	shutdownAnimationHandler();
	shutdownTimer();
	popTextureMemoryStack();
	popMemoryStack();
}

static void updateScreen(Screen* tScreen) {
	updateInput();
	updatePhysicsHandler();
	updateAnimationHandler();
	updateStageHandler();
	updateCollisionHandler();
	updateTimer();

	tScreen->mUpdate();
}

static void drawScreen(Screen* tScreen) {
	drawHandledAnimations();

	tScreen->mDraw();
}

static Screen* showScreen(Screen* tScreen) {

	Screen* next = NULL;
	while(next == NULL) {
		updateScreen(tScreen);
		drawScreen(tScreen);
		next = tScreen->mGetNextScreen();	
	}

	return next;
}


void startScreenHandling(Screen* tScreen) {
	gData.mIsAborted = 0;

	while(!gData.mIsAborted) {
		loadScreen(tScreen);
		Screen* next = showScreen(tScreen);
		unloadScreen(tScreen);
		tScreen = next;
	}
	
}
