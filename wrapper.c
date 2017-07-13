#include "tari/wrapper.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "tari/pvr.h"
#include "tari/physics.h"
#include "tari/file.h"
#include "tari/drawing.h"
#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/sound.h"

#include "tari/timer.h"
#include "tari/animation.h"
#include "tari/input.h"
#include "tari/physicshandler.h"
#include "tari/stagehandler.h"
#include "tari/collisionhandler.h"
#include "tari/collisionanimation.h"
#include "tari/soundeffect.h"
#include "tari/system.h"
#include "tari/texturepool.h"
#include "tari/texthandler.h"
#include "tari/screeneffect.h"
#include "tari/actorhandler.h"
#include "tari/tweening.h"

static struct {
	int mIsAborted;
	Screen* mNext;
	Screen* mScreen;
	int mIsPaused;
} gData;

void initTariWrapperWithDefaultFlags() {
	logg("Initiating system.");
	initSystem();
	logg("Initiating PowerVR.");
	initiatePVR();
	logg("Initiating memory handler.");
	initMemoryHandler();
	logg("Initiating physics.");
	initPhysics();
	logg("Initiating file system.");
	initFileSystem();
	logg("Initiating drawing.");
	initDrawing();
	logg("Initiating sound.");
	initSound();
	logg("Initiating sound effects.");
	initSoundEffects();
	logg("Initiating font.");
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
	logg("Initiating screen effects.");
	initScreenEffects();
	
}
void shutdownTariWrapper() {
	shutdownSound();
	shutdownMemoryHandler();
	shutdownSystem();
}

void pauseWrapper() {
	if (gData.mIsPaused) return;
	pausePhysics();
	pauseDurationHandling();
	gData.mIsPaused = 1;
}

void resumeWrapper() {
	if (!gData.mIsPaused) return;
	resumePhysics();
	resumeDurationHandling();
	gData.mIsPaused = 0;
}

int isWrapperPaused()
{
	return gData.mIsPaused;
}



static void loadScreen(Screen* tScreen) {
	gData.mScreen = tScreen;
	gData.mNext = NULL;

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
	logg("Setting up Tweeninghandling");
	setupTweening();
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
	logg("Setting up Actorhandling");
	setupActorHandler();
	logg("Setting up input flanks");
	resetInput();
	enableDrawing();	

	if (tScreen->mLoad) {
		logg("Loading user screen data");
		tScreen->mLoad();
	}
}

static void unloadScreen(Screen* tScreen) {
	logg("Unloading handled screen");
	
	if (tScreen->mUnload) {
		logg("Unloading user screen data");
		tScreen->mUnload();
	}

	logg("Shutting down Actorhandling");
	shutdownActorHandler();
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
	logg("Shutting down Tweeninghandling");
	shutdownTweening();
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

static void updateScreen() {
	updateSystem();
	updateInput();
	updatePhysicsHandler();
	updateTweening();
	updateAnimationHandler();
	updateTextHandler();
	updateStageHandler();
	updateCollisionAnimationHandler();
	updateCollisionHandler();
	updateTimer();
	updateActorHandler();

	if (gData.mScreen->mUpdate) {
		gData.mScreen->mUpdate();
	}
}

static void drawScreen() {
	waitForScreen();
	startDrawing();
	drawHandledAnimations();
	drawHandledTexts();
	drawHandledCollisions();
	drawActorHandler();

	if (gData.mScreen->mDraw) {
		gData.mScreen->mDraw();
	}

	stopDrawing();
}

static void performScreenIteration() {
	updateScreen();
	drawScreen();
	if (gData.mScreen->mGetNextScreen && !gData.mNext) {
		gData.mNext = gData.mScreen->mGetNextScreen();
	}
	
	// TODO: make Emscripten less hacky
#ifdef __EMSCRIPTEN__
	if (gData.mIsAborted || gData.mNext != NULL) {
		emscripten_cancel_main_loop();
		unloadScreen(gData.mScreen);
		if (!gData.mIsAborted) {
			startScreenHandling(gData.mNext);
		}
		else {
			startScreenHandling(gData.mScreen);
		}
	}
#endif
}

static Screen* showScreen() {
	logg("Show screen");

	// TODO: make Emscripten less hacky
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(performScreenIteration, 0, 1);
#endif

	while(!gData.mIsAborted && gData.mNext == NULL) {
		performScreenIteration();
	}

	return gData.mNext;
}

void abortScreenHandling() {
	gData.mIsAborted = 1;
}

void setNewScreen(Screen * tScreen)
{
	gData.mNext = tScreen;
}

void startScreenHandling(Screen* tScreen) {
	gData.mIsAborted = 0;
	while(!gData.mIsAborted) {
		loadScreen(tScreen);
		Screen* next = showScreen();
		unloadScreen(tScreen);
		tScreen = next;
	}
	
}
