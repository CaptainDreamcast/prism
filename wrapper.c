#include "prism/wrapper.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "prism/pvr.h"
#include "prism/physics.h"
#include "prism/file.h"
#include "prism/drawing.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/sound.h"

#include "prism/timer.h"
#include "prism/animation.h"
#include "prism/input.h"
#include "prism/physicshandler.h"
#include "prism/stagehandler.h"
#include "prism/collisionhandler.h"
#include "prism/collisionanimation.h"
#include "prism/soundeffect.h"
#include "prism/system.h"
#include "prism/texturepool.h"
#include "prism/texthandler.h"
#include "prism/screeneffect.h"
#include "prism/actorhandler.h"
#include "prism/tweening.h"
#include "prism/mugendefreader.h"
#include "prism/mugentexthandler.h"
#include "prism/mugenanimationhandler.h"
#include "prism/clipboardhandler.h"
#include "prism/blitzentity.h"
#include "prism/blitzmugenanimation.h"
#include "prism/blitzmugensound.h"
#include "prism/blitzphysics.h"
#include "prism/blitzcamerahandler.h"
#include "prism/blitztimelineanimation.h"
#include "prism/blitzcollision.h"
#include "prism/blitzparticles.h"
#include "prism/wrappercomponenthandler.h"
#include "prism/thread.h"
#include "prism/loadingscreen.h"

static struct {
	int mIsAborted;
	Screen* mNext;
	Screen* mScreen;
	Screen* mTitleScreen;
	int mIsPaused;
	int mHasFinishedLoading;

	double mUpdateTimeCounter;
	double mGlobalTimeDilatation;

	int mIsUsingBasicTextHandler;
	int mIsUsingMugen;
	int mIsUsingClipboard;
	int mIsUsingBlitzModule;

	int mHasBetweenScreensCB;
	void(*mBetweenScreensCB)(void*);
	void* mBetweenScreensCaller;

	int mIsNotPausingTracksBetweenScreens;
} gData;

static void initBasicSystems() {
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
	logg("Initiating screen effects.");
	initScreenEffects();
	logg("Initiating threading.");
	initThreading();

	gData.mGlobalTimeDilatation = 1;
}

void initPrismWrapperWithDefaultFlags() {
	initBasicSystems();
	logg("Initiating font.");
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
	
	gData.mIsUsingBasicTextHandler = 1;
}

void initPrismWrapperWithConfigFile(char* tPath) {
	initBasicSystems();

	MugenDefScript configFile = loadMugenDefScript(tPath);
	gData.mIsUsingBasicTextHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "texthandler", 0);
	gData.mIsUsingMugen = getMugenDefIntegerOrDefault(&configFile, "Modules", "mugen", 0);
	if (gData.mIsUsingMugen) {
		logg("Setting up Mugen Module for game");
		loadMugenTextHandler();
	}
	gData.mIsUsingClipboard = getMugenDefIntegerOrDefault(&configFile, "Modules", "clipboard", 0);
	if (gData.mIsUsingClipboard) {
		logg("Setting up Clipboard for game");
		char* fontName = getAllocatedMugenDefStringVariable(&configFile, "Clipboard", "font");
		addMugenFont(-1, fontName);
		freeMemory(fontName);
		initClipboardForGame();
	}
	gData.mIsUsingBlitzModule = getMugenDefIntegerOrDefault(&configFile, "Modules", "blitz", 0);

	unloadMugenDefScript(configFile);
}



void shutdownPrismWrapper() {
	shutdownThreading();
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

static void resumeWrapperComponentsForced() {
	resumePhysics();
	resumeDurationHandling();
	gData.mIsPaused = 0;
}

void resumeWrapper() {
	if (!gData.mIsPaused) return;
	resumeWrapperComponentsForced();
}

int isWrapperPaused()
{
	return gData.mIsPaused;
}

static void loadingThreadFunction(void* tCaller) {
	Screen* tScreen = tCaller;

	if (tScreen->mLoad) {
		logg("Loading user screen data");
		tScreen->mLoad();
	}

	gData.mHasFinishedLoading = 1;
}

static void loadScreen(Screen* tScreen) {
	gData.mScreen = tScreen;
	gData.mNext = NULL;

	logg("Loading handled screen");
	logg("Pushing memory stacks");
	pushMemoryStack();
	pushTextureMemoryStack();

	logFormat("Blocks allocated pre-screen: %d.", getAllocatedMemoryBlockAmount());

	logg("Setting up wrapper component handler");
	setupWrapperComponentHandler();
	logg("Setting up Timer");
	setupTimer();
	logg("Setting up Texture pool");
	setupTexturePool();
	logg("Setting up Animationhandling");
	setupAnimationHandler();
	logg("Setting up Tweeninghandling");
	setupTweening();
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

	

	if (gData.mIsUsingBasicTextHandler) {
		logg("Setting up Texthandling");
		addWrapperComponent(TextHandler);
	}

	if (gData.mIsUsingMugen) {
		logg("Setting up Mugen Module");
		addWrapperComponent(MugenAnimationHandler);
		addWrapperComponent(MugenTextHandler);
	}

	if (gData.mIsUsingClipboard) {
		logg("Setting up Clipboard");
		addWrapperComponent(ClipboardHandler);
	}
	if (gData.mIsUsingBlitzModule) {
		logg("Setting up Blitz Module");
		addWrapperComponent(BlitzCameraHandler);
		addWrapperComponent(BlitzParticleHandler);
		addWrapperComponent(BlitzEntityHandler);
		addWrapperComponent(BlitzMugenAnimationHandler);
		addWrapperComponent(BlitzMugenSoundHandler);
		addWrapperComponent(BlitzCollisionHandler);
		addWrapperComponent(BlitzPhysicsHandler);
		addWrapperComponent(BlitzTimelineAnimationHandler);
	}

	logg("Setting up input flanks");
	resetInputForAllControllers();
	enableDrawing();

	gData.mHasFinishedLoading = 0;
	int hasLoadingScreen = 0;// TODO
	if(hasLoadingScreen) {	
		startThread(loadingThreadFunction, tScreen);
		logg("Start loading screen");
		startLoadingScreen(&gData.mHasFinishedLoading);
		logg("End loading screen");
	} else {
		loadingThreadFunction(tScreen);
	}

	
}

static void callBetweenScreensCB() {
	if (!gData.mHasBetweenScreensCB) return;

	logg("Calling user CB");
	gData.mBetweenScreensCB(gData.mBetweenScreensCaller);
	gData.mHasBetweenScreensCB = 0;
}

static void unloadScreen(Screen* tScreen) {
	logg("Unloading handled screen");
	
	if (tScreen->mUnload) {
		logg("Unloading user screen data");
		tScreen->mUnload();
	}

	if (!gData.mIsNotPausingTracksBetweenScreens) {
		stopTrack();
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
	logg("Shutting down Tweeninghandling");
	shutdownTweening();
	logg("Shutting down Animationhandling");
	shutdownAnimationHandler();
	logg("Shutting down Texture pool");
	shutdownTexturePool();
	logg("Shutting down Timer");
	shutdownTimer();
	logg("Shutting down Wrapper component handler");
	shutdownWrapperComponentHandler();

	logFormat("Blocks allocated post-screen: %d.", getAllocatedMemoryBlockAmount());

	logg("Popping Memory Stacks");
	popTextureMemoryStack();
	popMemoryStack();

	callBetweenScreensCB();

	logMemoryState();
	logTextureMemoryState();
}

static void updateScreenAbort() {
	if (hasPressedAbortFlank()) {
		if (!gData.mTitleScreen || gData.mScreen == gData.mTitleScreen) {
			abortScreenHandling();
		}
		else {
			setNewScreen(gData.mTitleScreen);
		}
	}

}

static void updateScreen() {
	updateSystem();
	updateInput();
	updatePhysicsHandler();
	updateTweening();
	updateAnimationHandler();
	updateStageHandler();
	updateCollisionAnimationHandler();
	updateCollisionHandler();
	updateTimer();
	updateWrapperComponentHandler();
	updateActorHandler();

	if (gData.mScreen->mUpdate) {
		gData.mScreen->mUpdate();
	}

	updateScreenAbort();
}

static void drawScreen() {
	waitForScreen();
	startDrawing();
	drawHandledAnimations();
	drawHandledCollisions();
	drawWrapperComponentHandler();
	drawActorHandler();

	if (gData.mScreen->mDraw) {
		gData.mScreen->mDraw();
	}

	stopDrawing();
}

static void performScreenIteration() {
	gData.mUpdateTimeCounter += gData.mGlobalTimeDilatation;

	int updateAmount = (int)gData.mUpdateTimeCounter;
	int i;
	for (i = 0; i < updateAmount; i++) {
		updateScreen();
		gData.mUpdateTimeCounter = 0;
	}
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
	emscripten_set_main_loop(performScreenIteration, 60, 1);
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
		resumeWrapperComponentsForced();
		tScreen = next;
	}
	
}



void setWrapperTimeDilatation(double tDilatation)
{
	gData.mGlobalTimeDilatation = tDilatation;
}

void setWrapperBetweenScreensCB(void(*tCB)(void *), void* tCaller)
{
	gData.mBetweenScreensCB = tCB;
	gData.mBetweenScreensCaller = tCaller;
	gData.mHasBetweenScreensCB = 1;
}

void setWrapperIsPausingTracksBetweenScreens(int tIsPausingTracks)
{
	gData.mIsNotPausingTracksBetweenScreens = !tIsPausingTracks;
}

void setWrapperTitleScreen(Screen * tTitleScreen)
{
	gData.mTitleScreen = tTitleScreen;
}
