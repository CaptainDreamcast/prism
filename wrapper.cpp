#include "prism/wrapper.h"

#include <setjmp.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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
#include "prism/thread.h"
#include "prism/loadingscreen.h"
#include "prism/errorscreen.h"
#include "prism/debug.h"


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
	int mIsUsingStageHandler;
	int mIsUsingCollisionAnimationHandler;

	int mHasBetweenScreensCB;
	void(*mBetweenScreensCB)(void*);
	void* mBetweenScreensCaller;

	int mIsNotPausingTracksBetweenScreens;
	int mIsInExhibitionMode;
	Exhibition mExhibition;

	WrapperDebug mDebug;
	jmp_buf mExceptionJumpBuffer;

	int mIsNotUsingWrapperRecovery;
	int mIsActive;
} gPrismWrapperData;

static void initBasicSystems() {
	logg("Initiating system.");
	initSystem();
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
	if (isInDevelopMode()) {
		debugLog("Initiating debug.");
		initDebug();
	}


	gPrismWrapperData.mGlobalTimeDilatation = 1;
	gPrismWrapperData.mIsActive = 1;
}

void initPrismWrapperWithDefaultFlags() {
	initBasicSystems();
	debugLog("Initiating font.");
	setFont("$/rd/fonts/dolmexica.hdr", "$/rd/fonts/dolmexica.pkg");
	
	gPrismWrapperData.mIsUsingBasicTextHandler = 1;
}

void initPrismWrapperWithMugenFlags() {
	initBasicSystems();

	gPrismWrapperData.mIsUsingMugen = 1;
	gPrismWrapperData.mIsUsingClipboard = 1;

	debugLog("Initiating mugen text handler.");
	loadMugenTextHandler();
}

void initPrismWrapperWithConfigFile(const char* tPath) {
	initBasicSystems();

	MugenDefScript configFile;
	loadMugenDefScript(&configFile, tPath);
	gPrismWrapperData.mIsUsingBasicTextHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "texthandler", 0);
	gPrismWrapperData.mIsUsingStageHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "stagehandler", 0);
	gPrismWrapperData.mIsUsingCollisionAnimationHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "collisionanimationhandler", 0);
	gPrismWrapperData.mIsUsingMugen = getMugenDefIntegerOrDefault(&configFile, "Modules", "mugen", 0);
	if (gPrismWrapperData.mIsUsingMugen) {
		debugLog("Setting up Mugen Module for game");
		loadMugenTextHandler();
	}
	gPrismWrapperData.mIsUsingClipboard = getMugenDefIntegerOrDefault(&configFile, "Modules", "clipboard", 0);
	if (gPrismWrapperData.mIsUsingClipboard) {
		debugLog("Setting up Clipboard for game");
		char* fontName = getAllocatedMugenDefStringVariable(&configFile, "Clipboard", "font");
		addMugenFont(-1, fontName);
		freeMemory(fontName);
		initClipboardForGame();
	}
	gPrismWrapperData.mIsUsingBlitzModule = getMugenDefIntegerOrDefault(&configFile, "Modules", "blitz", 0);

	unloadMugenDefScript(configFile);
}

void shutdownPrismWrapper() {
	shutdownThreading();
	shutdownSound();
	shutdownMemoryHandler();
	shutdownSystem();

	gPrismWrapperData.mIsActive = 0;
}

void pauseWrapper() {
	if (gPrismWrapperData.mIsPaused) return;
	pausePhysics();
	pauseDurationHandling();
	gPrismWrapperData.mIsPaused = 1;
}

static void resumeWrapperComponentsForced() {
	resumePhysics();
	resumeDurationHandling();
	gPrismWrapperData.mIsPaused = 0;
}

void resumeWrapper() {
	if (!gPrismWrapperData.mIsPaused) return;
	resumeWrapperComponentsForced();
}

int isWrapperPaused()
{
	return gPrismWrapperData.mIsPaused;
}

int isUsingWrapper()
{
	return gPrismWrapperData.mIsActive;
}

static void loadingThreadFunction(void* tCaller) {
	Screen* tScreen = (Screen*)tCaller;

	if (tScreen->mLoad) {
		debugLog("Loading user screen data");
		tScreen->mLoad();
	}

	gPrismWrapperData.mHasFinishedLoading = 1;
}

static void loadScreen(Screen* tScreen) {
	gPrismWrapperData.mScreen = tScreen;
	gPrismWrapperData.mNext = NULL;
	gPrismWrapperData.mHasFinishedLoading = 0;

	logg("Loading handled screen");
	debugLog("Pushing memory stacks");
	pushMemoryStack();
	pushTextureMemoryStack();

	logFormat("Blocks allocated pre-screen: %d.", getAllocatedMemoryBlockAmount());

	debugLog("Setting up Timer");
	setupTimer();
	debugLog("Setting up Texture pool");
	setupTexturePool();
	debugLog("Setting up Animationhandling");
	setupAnimationHandler();
	debugLog("Setting up Physicshandling");
	setupPhysicsHandler();
	debugLog("Setting up Collisionhandling");
	setupCollisionHandler();
	debugLog("Setting up Soundeffecthandling");
	setupSoundEffectHandler();
	debugLog("Setting up Actorhandling");
	setupActorHandler();
	debugLog("Setting up Tweeninghandling");
	instantiateActor(getTweeningHandler());
	debugLog("Setting up Screen effects");
	instantiateActor(getScreenEffectHandler());

	if (gPrismWrapperData.mIsUsingBasicTextHandler) {
		debugLog("Setting up Texthandling");
		instantiateActor(getTextHandler());
	}

	if (gPrismWrapperData.mIsUsingStageHandler) {
		debugLog("Setting up Stagehandling");
		instantiateActor(getStageHandler());
	}

	if (gPrismWrapperData.mIsUsingCollisionAnimationHandler) {
		debugLog("Setting up Collisionanimationhandling");
		instantiateActor(getCollisionAnimationHandler());
	}

	if (gPrismWrapperData.mIsUsingMugen) {
		debugLog("Setting up Mugen Module");
		instantiateActor(getMugenAnimationHandler());
		instantiateActor(getMugenTextHandler());
	}

	if (gPrismWrapperData.mIsUsingClipboard) {
		debugLog("Setting up Clipboard");
		instantiateActor(getClipboardHandler());
	}
	if (gPrismWrapperData.mIsUsingBlitzModule) {
		debugLog("Setting up Blitz Module");
		instantiateActor(getBlitzCameraHandler());
		instantiateActor(getBlitzParticleHandler());
		instantiateActor(getBlitzEntityHandler());
		instantiateActor(getBlitzMugenAnimationHandler());
		instantiateActor(getBlitzMugenSoundHandler());
		instantiateActor(getBlitzCollisionHandler());
		instantiateActor(getBlitzPhysicsHandler());
		instantiateActor(getBlitzTimelineAnimationHandler());
	}

	if (isInDevelopMode()) {
		instantiateActor(getPrismDebug());
	}

	debugLog("Setting up input flanks");
	resetInputForAllControllers();
	enableDrawing();

	int hasLoadingScreen = isOnDreamcast();
	if(hasLoadingScreen) {	
		startThread(loadingThreadFunction, tScreen);
		debugLog("Start loading screen");
		startLoadingScreen(&gPrismWrapperData.mHasFinishedLoading);
		debugLog("End loading screen");
	} else {
		loadingThreadFunction(tScreen);
	}

	
}

static void callBetweenScreensCB() {
	if (!gPrismWrapperData.mHasBetweenScreensCB) return;

	debugLog("Calling user CB");
	gPrismWrapperData.mBetweenScreensCB(gPrismWrapperData.mBetweenScreensCaller);
	gPrismWrapperData.mHasBetweenScreensCB = 0;
}

static void stopWrapperMusic() {
	if (!gPrismWrapperData.mIsNotPausingTracksBetweenScreens) {
		stopMusic();
	}
}

static void unloadWrapper() {
	stopWrapperMusic();

	debugLog("Shutting down Actorhandling");
	shutdownActorHandler();
	debugLog("Shutting down Soundeffecthandling");
	shutdownSoundEffectHandler();
	debugLog("Shutting down Collisionhandling");
	shutdownCollisionHandler();
	debugLog("Shutting down Physicshandling");
	shutdownPhysicsHandler();
	debugLog("Shutting down Animationhandling");
	shutdownAnimationHandler();
	debugLog("Shutting down Texture pool");
	shutdownTexturePool();
	debugLog("Shutting down Timer");
	shutdownTimer();

	logFormat("Blocks allocated post-screen: %d.", getAllocatedMemoryBlockAmount());

	debugLog("Popping Memory Stacks");
	popTextureMemoryStack();
	popMemoryStack();

	callBetweenScreensCB();

	logMemoryState();
	logTextureMemoryState();
}

static void unloadScreen(Screen* tScreen) {
	logg("Unloading handled screen");
	stopWrapperMusic();

	if (tScreen->mUnload) {
		debugLog("Unloading user screen data");
		tScreen->mUnload();
	}

	unloadWrapper();
}

static void updateScreenDebug() {
	if (!isInDevelopMode()) return;

	if (hasPressedKeyboardKeyFlank(KEYBOARD_PAUSE_PRISM)) {
		gPrismWrapperData.mDebug.mIsPaused = gPrismWrapperData.mDebug.mIsPaused ? 0 : 2;
	}

	if (gPrismWrapperData.mDebug.mIsPaused == 1) gPrismWrapperData.mDebug.mIsPaused++;
	if (gPrismWrapperData.mDebug.mIsPaused && hasPressedKeyboardKeyFlank(KEYBOARD_SCROLLLOCK_PRISM)) {
		gPrismWrapperData.mDebug.mIsPaused = 1;
	}

	if (hasPressedKeyboardKeyFlank(KEYBOARD_R_PRISM)) {
		setNewScreen(gPrismWrapperData.mScreen);
	}
}

static void updateExhibitionMode() {
	if (!gPrismWrapperData.mIsInExhibitionMode) return;
	if (!gPrismWrapperData.mTitleScreen || gPrismWrapperData.mScreen == gPrismWrapperData.mTitleScreen) return;

	if (hasPressedAnyButton()) {
		gPrismWrapperData.mExhibition.mTicksSinceInput = 0;
	}

	int secondsSinceInput = gPrismWrapperData.mExhibition.mTicksSinceInput / (getFramerate() * 60);
	if (secondsSinceInput >= 2) {
		setNewScreen(gPrismWrapperData.mTitleScreen);
		gPrismWrapperData.mExhibition.mTicksSinceInput = 0;
	}
}

static void updateScreenAbort() {
	if (hasPressedAbortFlank()) {
		if ((!gPrismWrapperData.mTitleScreen || gPrismWrapperData.mScreen == gPrismWrapperData.mTitleScreen) && !gPrismWrapperData.mIsInExhibitionMode) {
			abortScreenHandling();
		}
		else {
			setNewScreen(gPrismWrapperData.mTitleScreen);
		}
	}
}



static void updateScreen() {
	updateSystem();
	updateInput();

	if (gPrismWrapperData.mDebug.mIsPaused < 2 && !gPrismWrapperData.mIsPaused) {
		updatePhysicsHandler();
		updateAnimationHandler();
		updateCollisionHandler();
		updateTimer();
	}

	updateActorHandler();
	
	if (gPrismWrapperData.mDebug.mIsPaused < 2 && !gPrismWrapperData.mIsPaused) {
		if (gPrismWrapperData.mScreen->mUpdate) {
			gPrismWrapperData.mScreen->mUpdate();
		}
	}

	updateScreenDebug();
	updateExhibitionMode();
	updateScreenAbort();
}

static void drawScreen() {
	setPrismDebugWaitingStartTime();
	waitForScreen();

	setPrismDebugDrawingStartTime();
	startDrawing();
	drawHandledAnimations();
	drawHandledCollisions();
	drawActorHandler();

	if (gPrismWrapperData.mScreen->mDraw) {
		gPrismWrapperData.mScreen->mDraw();
	}

	stopDrawing();
}

static void performScreenIteration() {
	setPrismDebugUpdateStartTime();

	gPrismWrapperData.mUpdateTimeCounter += gPrismWrapperData.mGlobalTimeDilatation;
	int updateAmount = (int)gPrismWrapperData.mUpdateTimeCounter;
	int i;
	for (i = 0; i < updateAmount; i++) {
		updateScreen();
	}
	gPrismWrapperData.mUpdateTimeCounter -= updateAmount;

	drawScreen();
	if (gPrismWrapperData.mScreen->mGetNextScreen && !gPrismWrapperData.mNext) {
		gPrismWrapperData.mNext = gPrismWrapperData.mScreen->mGetNextScreen();
	}
	
#ifdef __EMSCRIPTEN__
	if (gPrismWrapperData.mIsAborted || gPrismWrapperData.mNext != NULL) {
		emscripten_cancel_main_loop();
		unloadScreen(gPrismWrapperData.mScreen);
		if (!gPrismWrapperData.mIsAborted) {
			startScreenHandling(gPrismWrapperData.mNext);
		}
		else {
			startScreenHandling(gPrismWrapperData.mScreen);
		}
	}
#endif
}

static Screen* showScreen() {
	logg("Show screen");

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(performScreenIteration, 60, 1);
#endif

	while(!gPrismWrapperData.mIsAborted && gPrismWrapperData.mNext == NULL) {
		performScreenIteration();
	}

	return gPrismWrapperData.mNext;
}

void abortScreenHandling() {
	gPrismWrapperData.mIsAborted = 1;
}

void setNewScreen(Screen * tScreen)
{
	gPrismWrapperData.mNext = tScreen;
}

Screen makeScreen(LoadScreenFunction tLoad, UpdateScreenFunction tUpdate, DrawScreenFunction tDraw, UnloadScreenFunction tUnload, Screen*(*tGetNextScreen)()) {
	Screen ret;
	ret.mLoad = tLoad;
	ret.mUpdate = tUpdate;
	ret.mDraw = tDraw;
	ret.mUnload = tUnload;
	ret.mGetNextScreen = tGetNextScreen;
	return ret;
}

void startScreenHandling(Screen* tScreen) {
	gPrismWrapperData.mIsAborted = 0;
// TODO: refactor with exceptions (https://dev.azure.com/captdc/DogmaRnDA/_workitems/edit/431)
#ifdef DREAMCAST
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4611)
#endif
	if (setjmp(gPrismWrapperData.mExceptionJumpBuffer)) {
		tScreen = gPrismWrapperData.mNext;
	}
#ifdef _WIN32
#pragma warning(pop)
#endif
	while(!gPrismWrapperData.mIsAborted) {
		loadScreen(tScreen);
		Screen* next = showScreen();
		unloadScreen(tScreen);
		resumeWrapperComponentsForced();
		tScreen = next;
	}
	
}



void setWrapperTimeDilatation(double tDilatation)
{
	gPrismWrapperData.mGlobalTimeDilatation = tDilatation;
}

void setWrapperBetweenScreensCB(void(*tCB)(void *), void* tCaller)
{
	gPrismWrapperData.mBetweenScreensCB = tCB;
	gPrismWrapperData.mBetweenScreensCaller = tCaller;
	gPrismWrapperData.mHasBetweenScreensCB = 1;
}

void setWrapperIsPausingTracksBetweenScreens(int tIsPausingTracks)
{
	gPrismWrapperData.mIsNotPausingTracksBetweenScreens = !tIsPausingTracks;
}

void setWrapperToExhibitionMode() {
	gPrismWrapperData.mExhibition.mTicksSinceInput = 0;
	gPrismWrapperData.mIsInExhibitionMode = 1;
}

void setWrapperTitleScreen(Screen * tTitleScreen)
{
	gPrismWrapperData.mTitleScreen = tTitleScreen;
}

void recoverWrapperError()
{
	gPrismWrapperData.mHasFinishedLoading = 1;

	if (gPrismWrapperData.mIsUsingMugen && !gPrismWrapperData.mIsNotUsingWrapperRecovery) {
		setNewScreen(getErrorScreen());
		unloadWrapper();
		longjmp(gPrismWrapperData.mExceptionJumpBuffer, 1);
	}
	else {
		gotoNextScreenAfterWrapperError();
	}

}

void gotoNextScreenAfterWrapperError()
{
	if (!gPrismWrapperData.mTitleScreen || gPrismWrapperData.mScreen == gPrismWrapperData.mTitleScreen || gPrismWrapperData.mIsNotUsingWrapperRecovery) {
		abortSystem();
	}
	else {
		setNewScreen(gPrismWrapperData.mTitleScreen);
		unloadWrapper();
		longjmp(gPrismWrapperData.mExceptionJumpBuffer, 1);
	}
}

void disableWrapperErrorRecovery()
{
	gPrismWrapperData.mIsNotUsingWrapperRecovery = 1;
}

