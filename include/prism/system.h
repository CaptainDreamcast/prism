#pragma once

#include <stdint.h>

#ifdef _WIN32
#define PERFORMANCE_FACTOR_INVERTED 1

#elif defined __EMSCRIPTEN__
#define PERFORMANCE_FACTOR_INVERTED 5

#elif defined DREAMCAST
#define PERFORMANCE_FACTOR_INVERTED 10

#endif 

#define PERFORMANCE_FACTOR (1.0/PERFORMANCE_FACTOR_INVERTED)

typedef struct {
	int x;
	int y;
} ScreenSize;

void initSystem();
void shutdownSystem();
void updateSystem();
void abortSystem();
void recoverFromError();
void returnToMenu();

void setGameName(char* tName);
void updateGameName(char* tName);

void setScreen(int tX, int tY, int tFramerate, int tIsVGA);
void setScreenSize(int tX, int tY);
ScreenSize getScreenSize();
void setDisplayedScreenSize(int tX, int tY);
void setScreenFramerate(int tFramerate);
void setVGA();
int isOnDreamcast();
int isOnWindows();
int isOnWeb();


uint64_t getSystemTicks();