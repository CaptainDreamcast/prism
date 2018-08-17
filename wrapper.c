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

#include "prism/framerate.h"
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

typedef struct {
	int mIsPaused;

} WrapperDebug;

typedef struct {
	int mTicksSinceInput;
} Exhibition;

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
	int mIsInExhibitionMode;
	Exhibition mExhibition;

	WrapperDebug mDebug;
} gData;

static void initBasicSystems() {
	logg("Initiating system.");
	initSystem();
	debugLog("Initiating PowerVR.");
	initiatePVR();
	debugLog("Initiating memory handler.");
	initMemoryHandler();
	debugLog("Initiating physics.");
	initPhysics();
	debugLog("Initiating file system.");
	initFileSystem();
	debugLog("Initiating drawing.");
	initDrawing();
	debugLog("Initiating sound.");
	initSound();
	debugLog("Initiating sound effects.");
	initSoundEffects();
	debugLog("Initiating screen effects.");
	initScreenEffects();
	debugLog("Initiating threading.");
	initThreading();
	debugLog("Initiating input.");
	initInput();

	gData.mGlobalTimeDilatation = 1;
}

void initPrismWrapperWithDefaultFlags() {
	initBasicSystems();
	debugLog("Initiating font.");
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
	
	gData.mIsUsingBasicTextHandler = 1;
}

void initPrismWrapperWithConfigFile(char* tPath) {
	initBasicSystems();

	MugenDefScript configFile = loadMugenDefScript(tPath);
	gData.mIsUsingBasicTextHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "texthandler", 0);
	gData.mIsUsingMugen = getMugenDefIntegerOrDefault(&configFile, "Modules", "mugen", 0);
	if (gData.mIsUsingMugen) {
		debugLog("Setting up Mugen Module for game");
		loadMugenTextHandler();
	}
	gData.mIsUsingClipboard = getMugenDefIntegerOrDefault(&configFile, "Modules", "clipboard", 0);
	if (gData.mIsUsingClipboard) {
		debugLog("Setting up Clipboard for game");
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
		debugLog("Loading user screen data");
		tScreen->mLoad();
	}

	gData.mHasFinishedLoading = 1;
}

static void loadScreen(Screen* tScreen) {
	gData.mScreen = tScreen;
	gData.mNext = NULL;

	logg("Loading handled screen");
	debugLog("Pushing memory stacks");
	pushMemoryStack();
	pushTextureMemoryStack();

	logFormat("Blocks allocated pre-screen: %d.", getAllocatedMemoryBlockAmount());

	debugLog("Setting up wrapper component handler");
	setupWrapperComponentHandler();
	debugLog("Setting up Timer");
	setupTimer();
	debugLog("Setting up Texture pool");
	setupTexturePool();
	debugLog("Setting up Animationhandling");
	setupAnimationHandler();
	debugLog("Setting up Tweeninghandling");
	setupTweening();
	debugLog("Setting up Physicshandling");
	setupPhysicsHandler();
	debugLog("Setting up Stagehandling");
	setupStageHandler();
	debugLog("Setting up Collisionhandling");
	setupCollisionHandler();
	debugLog("Setting up Collisionanimationhandling");
	setupCollisionAnimationHandler();
	debugLog("Setting up Soundeffecthandling");
	setupSoundEffectHandler();
	debugLog("Setting up Actorhandling");
	setupActorHandler();

	

	if (gData.mIsUsingBasicTextHandler) {
		debugLog("Setting up Texthandling");
		addWrapperComponent(TextHandler);
	}

	if (gData.mIsUsingMugen) {
		debugLog("Setting up Mugen Module");
		addWrapperComponent(MugenAnimationHandler);
		addWrapperComponent(MugenTextHandler);
	}

	if (gData.mIsUsingClipboard) {
		debugLog("Setting up Clipboard");
		addWrapperComponent(ClipboardHandler);
	}
	if (gData.mIsUsingBlitzModule) {
		debugLog("Setting up Blitz Module");
		addWrapperComponent(BlitzCameraHandler);
		addWrapperComponent(BlitzParticleHandler);
		addWrapperComponent(BlitzEntityHandler);
		addWrapperComponent(BlitzMugenAnimationHandler);
		addWrapperComponent(BlitzMugenSoundHandler);
		addWrapperComponent(BlitzCollisionHandler);
		addWrapperComponent(BlitzPhysicsHandler);
		addWrapperComponent(BlitzTimelineAnimationHandler);
	}

	debugLog("Setting up input flanks");
	resetInputForAllControllers();
	enableDrawing();

	gData.mHasFinishedLoading = 0;
	int hasLoadingScreen = 0;// TODO
	if(hasLoadingScreen) {	
		startThread(loadingThreadFunction, tScreen);
		debugLog("Start loading screen");
		startLoadingScreen(&gData.mHasFinishedLoading);
		debugLog("End loading screen");
	} else {
		loadingThreadFunction(tScreen);
	}

	
}

static void callBetweenScreensCB() {
	if (!gData.mHasBetweenScreensCB) return;

	debugLog("Calling user CB");
	gData.mBetweenScreensCB(gData.mBetweenScreensCaller);
	gData.mHasBetweenScreensCB = 0;
}

static void unloadScreen(Screen* tScreen) {
	logg("Unloading handled screen");
	
	if (tScreen->mUnload) {
		debugLog("Unloading user screen data");
		tScreen->mUnload();
	}

	if (!gData.mIsNotPausingTracksBetweenScreens) {
		stopTrack();
	}

	debugLog("Shutting down Actorhandling");
	shutdownActorHandler();
	debugLog("Shutting down Soundeffecthandling");
	shutdownSoundEffectHandler();
	debugLog("Shutting down Collisionanimationhandling");
	shutdownCollisionAnimationHandler();
	debugLog("Shutting down Collisionhandling");
	shutdownCollisionHandler();
	debugLog("Shutting down Stagehandling");
	shutdownStageHandler();
	debugLog("Shutting down Physicshandling");
	shutdownPhysicsHandler();
	debugLog("Shutting down Tweeninghandling");
	shutdownTweening();
	debugLog("Shutting down Animationhandling");
	shutdownAnimationHandler();
	debugLog("Shutting down Texture pool");
	shutdownTexturePool();
	debugLog("Shutting down Timer");
	shutdownTimer();
	debugLog("Shutting down Wrapper component handler");
	shutdownWrapperComponentHandler();

	logFormat("Blocks allocated post-screen: %d.", getAllocatedMemoryBlockAmount());

	debugLog("Popping Memory Stacks");
	popTextureMemoryStack();
	popMemoryStack();

	callBetweenScreensCB();

	logMemoryState();
	logTextureMemoryState();
}

static void updateScreenDebug() {
	if (hasPressedKeyboardKeyFlank(KEYBOARD_PAUSE_PRISM)) {
		gData.mDebug.mIsPaused = gData.mDebug.mIsPaused ? 0 : 2;
	}

	if (gData.mDebug.mIsPaused == 1) gData.mDebug.mIsPaused++;
	if (gData.mDebug.mIsPaused && hasPressedKeyboardKeyFlank(KEYBOARD_SCROLLLOCK_PRISM)) {
		gData.mDebug.mIsPaused = 1;
	}

}

static void updateExhibitionMode() {
	if (!gData.mIsInExhibitionMode) return;
	if (!gData.mTitleScreen || gData.mScreen == gData.mTitleScreen) return;

	if (hasPressedAnyButton()) {
		gData.mExhibition.mTicksSinceInput = 0;
	}

	int secondsSinceInput = gData.mExhibition.mTicksSinceInput / (getFramerate() * 60);
	if (secondsSinceInput >= 2) {
		setNewScreen(gData.mTitleScreen);
		gData.mExhibition.mTicksSinceInput = 0;
	}
}

static void updateScreenAbort() {
	if (hasPressedAbortFlank()) {
		if ((!gData.mTitleScreen || gData.mScreen == gData.mTitleScreen) && !gData.mIsInExhibitionMode) {
			abortScreenHandling();
		}
		else {
			setNewScreen(gData.mTitleScreen);
		}
	}

}

static void updatePausableScreen() {
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
}

static void updateScreen() {
	updateSystem();
	updateInput();

	if (gData.mDebug.mIsPaused < 2) {
		updatePausableScreen();
	}

	updateScreenDebug();
	updateExhibitionMode();
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

void setWrapperToExhibitionMode() {
	gData.mExhibition.mTicksSinceInput = 0;
	gData.mIsInExhibitionMode = 1;
}

void setWrapperTitleScreen(Screen * tTitleScreen)
{
	gData.mTitleScreen = tTitleScreen;
}
