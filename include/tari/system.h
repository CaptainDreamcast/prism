#ifndef TARI_SYSTEM
#define TARI_SYSTEM

#include "common/header.h"

#ifdef _WIN32
#define PERFORMANCE_FACTOR_INVERTED 1

#elif defined __EMSCRIPTEN__
#define PERFORMANCE_FACTOR_INVERTED 3

#elif defined DREAMCAST
#define PERFORMANCE_FACTOR_INVERTED 15

#endif 

#define PERFORMANCE_FACTOR (1.0/PERFORMANCE_FACTOR_INVERTED)

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
