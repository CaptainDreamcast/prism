#ifndef TARI_SYSTEM
#define TARI_SYSTEM

#include "common/header.h"

typedef struct {
	int x;
	int y;
} ScreenSize;

fup void initSystem();
fup void shutdownSystem();
fup void updateSystem();
fup void abortSystem();
fup void returnToMenu();

fup void setGameName(char* tName);

fup void setScreen(int tX, int tY, int tFramerate, int tIsVGA);
fup void setScreenSize(int tX, int tY);
fup ScreenSize getScreenSize();
fup void setScreenFramerate(int tFramerate);
fup void setVGA();

#endif
