#ifndef TARI_WRAPPER
#define TARI_WRAPPER

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

void initTariWrapperWithDefaultFlags();
void shutdownTariWrapper();
void pauseWrapper();
void resumeWrapper();

void loadScreenFunctionality();
void updateScreenFunctionality();
void drawScreenFunctionality();
void shutdownScreenFunctionality();

void startScreenHandling(Screen* tScreen);
void abortScreenHandling();

#endif 
