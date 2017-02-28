#ifndef TARI_SYSTEM
#define TARI_SYSTEM

typedef struct {
	int x;
	int y;
} ScreenSize;

void abortSystem();

void setScreen(int tX, int tY, int tFramerate, int tIsVGA);
void setScreenSize(int tX, int tY);
ScreenSize getScreenSize();
void setScreenFramerate(int tFramerate);
void setVGA();

#endif
