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

typedef enum {
	FIFTY_HERTZ = 50,
	SIXTY_HERTZ = 60
} Framerate;
#define FRAMERATE_AMOUNT 2

void initSystem();
void shutdownSystem();
void updateSystem();
void abortSystem();
void recoverFromError();
void returnToMenu();

void setGameName(const char* tName);
void updateGameName(const char* tName);
void setIcon(const char* tPath);

void setScreen(int tX, int tY, int tFramerate, int tIsVGA);
void setScreenSize(int tX, int tY);
ScreenSize getScreenSize();
ScreenSize getDisplayedScreenSize();
void setDisplayedScreenSize(int tX, int tY);
void setScreenFramerate(int tFramerate);

void setFramerate(Framerate tFramerate);
Framerate getFramerate();
double getFramerateFactor();
double getInverseFramerateFactor();

void setVGA();
int isOnDreamcast();
int isOnWindows();
int isOnWeb();
int isOnVita();

void setSystemExitDisabled(int tIsExitDisabled);

uint64_t getSystemTicks();