#include <prism/framerateselectscreen.h>
#include <prism/windowfocusscreen.h>
#include <prism/physics.h>
#include <prism/file.h>
#include <prism/drawing.h>
#include <prism/log.h>
#include <prism/wrapper.h>
#include <prism/system.h>
#include <prism/stagehandler.h>
#include <prism/mugentexthandler.h>
#include <prism/debug.h>

#include "gamescreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

#endif

#define DEVELOP

void exitGame() {
	shutdownPrismWrapper();

#ifdef DEVELOP
	if (isOnDreamcast()) {
		abortSystem();
	}
	else {
		returnToMenu();
	}
#else
	returnToMenu();
#endif
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	#ifdef DEVELOP
	setDevelopMode();
	#endif

	setGameName("TEMPLATE");
	setScreenSize(320, 240);
	
	initPrismWrapperWithConfigFile("data/config.cfg");
	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");

	addMugenFont(-1, "font/f4x6.fnt");
	
	logg("Check framerate");
	if (selectFramerate() == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}

	if(isInDevelopMode()) {
		disableWrapperErrorRecovery();	
		setMinimumLogType(LOG_TYPE_NORMAL);
	}
	else {
		setMinimumLogType(LOG_TYPE_NONE);
	}

	const auto startScreen = getGameScreen();
#ifdef __EMSCRIPTEN__
	setWindowFocusScreenNextScreen(startScreen);
	startScreenHandling(getWindowFocusScreen());
#else
	startScreenHandling(startScreen);
#endif

	exitGame();
	
	return 0;
}


