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
#include "prism/clipboardhandler.h"

static struct {
	int mIsAborted;
	Screen* mNext;
	Screen* mScreen;
	int mIsPaused;

	double mUpdateTimeCounter;
	double mGlobalTimeDilatation;

	int mIsUsingBasicTextHandler;
	int mIsUsingMugenTextHandler;
	int mIsUsingClipboard;

	int mHasBetweenScreensCB;
	void(*mBetweenScreensCB)(void*);
	void* mBetweenScreensCaller;
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
	gData.mIsUsingMugenTextHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "mugentexthandler", 0);
	if (gData.mIsUsingMugenTextHandler) {
		logg("Setting up M-Texthandler for game");
		loadMugenTextHandler();
	}

	gData.mIsUsingBasicTextHandler = getMugenDefIntegerOrDefault(&configFile, "Modules", "texthandler", 0);
	gData.mIsUsingClipboard = getMugenDefIntegerOrDefault(&configFile, "Modules", "clipboard", 0);

	if (gData.mIsUsingClipboard) {
		logg("Setting up Clipboard for game");
		char* fontName = getAllocatedMugenDefStringVariable(&configFile, "Clipboard", "font");
		addMugenFont(-1, fontName);
		freeMemory(fontName);
		initClipboardForGame();
	}

	unloadMugenDefScript(configFile);
}



void shutdownPrismWrapper() {
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
		instantiateActor(TextHandler);
	}

	if (gData.mIsUsingMugenTextHandler) {
		logg("Setting up M-Texthandler");
		instantiateActor(MugenTextHandler);
	}

	if (gData.mIsUsingClipboard) {
		logg("Setting up Clipboard");
		instantiateActor(ClipboardHandler);
	}

	logg("Setting up input flanks");
	resetInputForAllControllers();
	enableDrawing();	

	if (tScreen->mLoad) {
		logg("Loading user screen data");
		tScreen->mLoad();
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

	stopTrack();

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
	logg("Popping Memory Stacks");
	popTextureMemoryStack();
	popMemoryStack();

	callBetweenScreensCB();

	logMemoryState();
	logTextureMemoryState();
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
	updateActorHandler();

	if (gData.mScreen->mUpdate) {
		gData.mScreen->mUpdate();
	}
}

static void drawScreen() {
	waitForScreen();
	startDrawing();
	drawHandledAnimations();
	drawHandledCollisions();
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
