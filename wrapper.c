#include "include/wrapper.h"

#include "include/pvr.h"
#include "include/physics.h"
#include "include/file.h"
#include "include/drawing.h"
#include "include/log.h"
#include "include/memoryhandler.h"
#include "include/sound.h"

#include "include/timer.h"
#include "include/animation.h"
#include "include/input.h"
#include "include/physicshandler.h"
#include "include/stagehandler.h"
#include "include/collisionhandler.h"
#include "include/collisionanimation.h"
#include "include/soundeffect.h"
#include "include/system.h"
#include "include/texturepool.h"
#include "include/texthandler.h"
#include "include/screeneffect.h"

void initTariWrapperWithDefaultFlags() {
	logg("Initiating wrapper.");
	initSystem();
	initiatePVR();
	initMemoryHandler();
	initPhysics();
	initFileSystem();
	initDrawing();
	initSound();
	initSoundEffects();
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
	initScreenEffects();
	
}
void shutdownTariWrapper() {
	shutdownSound();
	shutdownMemoryHandler();
	shutdownSystem();
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
	logg("Loading handled screen");
	logg("Pushing memory stacks");
	pushMemoryStack();
	pushTextureMemoryStack();
	logg("Setting up Timer");
	setupTimer();
	logg("Setting up Texture pool");
	setupTexturePool();
	logg("Setting up Animationhandling");
	setupAnimationHandler();
	logg("Setting up Texthandling");
	setupTextHandler();
	logg("Setting up Physicshandling");
	setupPhysicsHandler();
	logg("Setting up Stagehandling");
	setupStageHandler();
	logg("Setting up Collisionhandling");
	setupCollisionHandler();
	logg("Setting up Collisionanimationhandling");
	setupCollisionAnimationHandler();
	logg("Setting up Soundeffecthandling");
	setupSoundEffectHandler();
	logg("Setting up input flanks");
	resetInput();
	
	logg("Loading user screen data");
	tScreen->mLoad();
}

static void unloadScreen(Screen* tScreen) {
	logg("Unloading handled screen");
	logg("Unloading user screen data");
	tScreen->mUnload();

	logg("Shutting down Soundeffecthandling");
	shutdownSoundEffectHandler();
	logg("Shutting down Collisionanimationhandling");
	shutdownCollisionAnimationHandler();
	logg("Shutting down Collisionhandling");
	shutdownCollisionHandler();
	logg("Shutting down Stagehandling");
	shutdownStageHandler();
	logg("Shutting down Physicshandling");
	shutdownPhysicsHandler();
	logg("Shutting down Texthandling");
	shutdownTextHandler();
	logg("Shutting down Animationhandling");
	shutdownAnimationHandler();
	logg("Shutting down Texture pool");
	shutdownTexturePool();
	logg("Shutting down Timer");
	shutdownTimer();
	logg("Popping Memory Stacks");
	popTextureMemoryStack();
	popMemoryStack();

	logMemoryState();
	logTextureMemoryState();
}

static void updateScreen(Screen* tScreen) {
	updateSystem();
	updateInput();
	updatePhysicsHandler();
	updateAnimationHandler();
	updateTextHandler();
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
	drawHandledTexts();
	drawHandledCollisions();

	tScreen->mDraw();

	stopDrawing();
}

static Screen* showScreen(Screen* tScreen) {
	logg("Show screen");

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
