#ifndef TARI_WRAPPER
#define TARI_WRAPPER

#include "common/header.h"

typedef void (*LoadScreenFunction)();
typedef void (*UpdateScreenFunction)();
typedef void (*DrawScreenFunction)();
typedef void (*UnloadScreenFunction)();

typedef struct Screen_internal{
	LoadScreenFunction mLoad;
	UpdateScreenFunction mUpdate;
	DrawScreenFunction mDraw;
	UnloadScreenFunction mUnload;
	struct Screen_internal* (*mGetNextScreen)();
} Screen;

fup void initTariWrapperWithDefaultFlags();
fup void shutdownTariWrapper();
fup void pauseWrapper();
fup void resumeWrapper();

fup void loadScreenFunctionality();
fup void updateScreenFunctionality();
fup void drawScreenFunctionality();
fup void shutdownScreenFunctionality();

fup void startScreenHandling(Screen* tScreen);
fup void abortScreenHandling();

#endif 
