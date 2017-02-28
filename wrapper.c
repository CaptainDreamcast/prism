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
#include "include/collisionanimation.h"
#include "include/system.h"

void initTariWrapperWithDefaultFlags() {
	log("Initiating wrapper.");
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

void pauseWrapper() {
	pausePhysics();
	pauseDurationHandling();
}

void resumeWrapper() {
	resumePhysics();
	resumeDurationHandling();
}

static struct {
	int mIsAborted;
} gData;

static void loadScreen(Screen* tScreen) {
	log("Loading handled screen");
	log("Pushing memory stacks");
	pushMemoryStack();
	pushTextureMemoryStack();
	log("Setting up Timer");
	setupTimer();
	log("Setting up Animationhandling");
	setupAnimationHandler();
	log("Setting up Physicshandling");
	setupPhysicsHandler();
	log("Setting up Stagehandling");
	setupStageHandler();
	log("Setting up Collisionhandling");
	setupCollisionHandler();
	log("Setting up Collisionanimationhandling");
	setupCollisionAnimationHandler();
	
	log("Loading user screen data");
	tScreen->mLoad();
}

static void unloadScreen(Screen* tScreen) {
	log("Unloading handled screen");
	log("Unloading user screen data");
	tScreen->mUnload();

	log("Shutting down Collisionanimationhandling");
	shutdownCollisionAnimationHandler();
	log("Shutting down Collisionhandling");
	shutdownCollisionHandler();
	log("Shutting down Stagehandling");
	shutdownStageHandler();
	log("Shutting down Physicshandling");
	shutdownPhysicsHandler();
	log("Shutting down Animationhandling");
	shutdownAnimationHandler();
	log("Shutting down Timer");
	shutdownTimer();
	log("Popping Memory Stacks");
	popTextureMemoryStack();
	popMemoryStack();

	logMemoryState();
	logTextureMemoryState();
}

static void updateScreen(Screen* tScreen) {
	updateInput();
	updatePhysicsHandler();
	updateAnimationHandler();
	updateStageHandler();
	updateCollisionAnimationHandler();
	updateCollisionHandler();
	updateTimer();

	tScreen->mUpdate();
}

static void drawScreen(Screen* tScreen) {
	waitForScreen();
	startDrawing();
	drawHandledAnimations();
	drawHandledCollisions();

	tScreen->mDraw();

	stopDrawing();
}

static Screen* showScreen(Screen* tScreen) {
	log("Show screen");

	Screen* next = NULL;
	while(!gData.mIsAborted && next == NULL) {
		updateScreen(tScreen);
		drawScreen(tScreen);
		next = tScreen->mGetNextScreen();	
	}

	return next;
}

void abortScreenHandling() {
	gData.mIsAborted = 1;
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
