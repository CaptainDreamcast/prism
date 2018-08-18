#pragma once

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

void initPrismWrapperWithDefaultFlags();
void initPrismWrapperWithoutMemoryManagement();
void initPrismWrapperWithConfigFile(char* tPath);
void shutdownPrismWrapper();
void pauseWrapper();
void resumeWrapper();
int isWrapperPaused();
int isUsingWrapper();

void startScreenHandling(Screen* tScreen);
void abortScreenHandling();
void setNewScreen(Screen* tScreen);

void setWrapperTimeDilatation(double tDilatation);
void setWrapperBetweenScreensCB(void(*tCB)(void*), void* tCaller);
void setWrapperIsPausingTracksBetweenScreens(int tIsPausingTracks);
void setWrapperToExhibitionMode();

void setWrapperTitleScreen(Screen* tTitleScreen);
void recoverWrapperError();