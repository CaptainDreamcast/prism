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
fup int isWrapperPaused();

fup void startScreenHandling(Screen* tScreen);
fup void abortScreenHandling();
fup void setNewScreen(Screen* tScreen);

void setWrapperTimeDilatation(double tDilatation);

#endif 
