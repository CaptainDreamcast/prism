#ifndef TARI_WRAPPER
#define TARI_WRAPPER

typedef void (*LoadScreenFunction)();
typedef void (*UpdateScreenFunction)();
typedef void (*GetNewScreenFunction)();
typedef void (*DrawScreenFunction)();
typedef void (*ShutdownScreenFunction)();

typedef struct Screen_internal{
	LoadScreenFunction mLoad;
	UpdateScreenFunction mUpdate;
	DrawScreenFunction mDraw;
	ShutdownScreenFunction mShutdown;
	struct Screen_internal* (*mGetScreen)();
} Screen;

void initTariWrapperWithDefaultFlags();
void shutdownTariWrapper();

void loadScreenFunctionality();
void updateScreenFunctionality();
void drawScreenFunctionality();
void shutdownScreenFunctionality();

void startScreenHandling(Screen* tScreen);

#endif 
