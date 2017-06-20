#include "tari/system.h"

#include <string.h>
#include <assert.h>

#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "tari/log.h"
#include "tari/pvr.h"
#include "tari/geometry.h"


void abortSystem(){
	assert(0);
	exit(0);
}	

static struct {

	int mIsLoaded;
	
	int mScreenSizeX;
	int mScreenSizeY;

	int mDisplayedWindowSizeX;
	int mDisplayedWindowSizeY;

	int mIsFullscreen;

	char mGameName[100];
} gData;

SDL_Window* gSDLWindow;

static void initScreenDefault() {
	gData.mIsLoaded = 1;
	gData.mScreenSizeX = gData.mDisplayedWindowSizeX = 640;
	gData.mScreenSizeY = gData.mDisplayedWindowSizeY = 480;

	gData.mIsFullscreen = 0;
}

void setGameName(char* tName) {
	strcpy(gData.mGameName, tName);
}


#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

static void setToProgramDirectory() {
#ifdef _WIN32
	TCHAR wbuf[1024];
	char buf[1024];
	GetModuleFileName(NULL, wbuf, 1024);

	int len = wcstombs(buf, wbuf, 1024);
	buf[len] = '\0';
	char* end = strrchr(buf, '\\');
	end[1] = '\0';

	_chdir(buf);
#endif
}

void initSystem() {

	setToProgramDirectory();
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	

	if (gData.mGameName[0] == '\0') {
		sprintf(gData.mGameName, "Unnamed libtari game port");
	}
	gSDLWindow = SDL_CreateWindow(gData.mGameName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, getScreenSize().x, getScreenSize().y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
}

void shutdownSystem() {
	SDL_DestroyWindow(gSDLWindow);

	IMG_Quit();
	SDL_Quit();
}

extern SDL_Renderer* gRenderer;

static void resizeWindow(SDL_Event* e) {
	if (gRenderer == NULL) return;

	ScreenSize sz = getScreenSize();
	gData.mDisplayedWindowSizeX = e->window.data1;
	gData.mDisplayedWindowSizeY = e->window.data2;

	double scaleX = gData.mDisplayedWindowSizeX /(double)sz.x;
	double scaleY = gData.mDisplayedWindowSizeY / (double)sz.y;

	SDL_RenderSetScale(gRenderer, (float)scaleX, (float)scaleY);
}
static void checkWindowEvents(SDL_Event* e) {
	if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
		resizeWindow(e);
	}
	
}

Vector3D correctSDLWindowPosition(Vector3D v) {
	ScreenSize sz = getScreenSize();
	double scaleX = gData.mDisplayedWindowSizeX / (double)sz.x;
	double scaleY = gData.mDisplayedWindowSizeY / (double)sz.y;
	return vecScale3D(v, makePosition(1 / scaleX, 1 / scaleY, 1));
}

static void switchFullscreen() {
	if (!gData.mIsFullscreen) {
		SDL_SetWindowFullscreen(gSDLWindow, SDL_WINDOW_FULLSCREEN);
	}
	else {
		SDL_SetWindowFullscreen(gSDLWindow, 0);
	}

	gData.mIsFullscreen ^= 1;
}

static void checkFullscreen() {
	const Uint8* kstates = SDL_GetKeyboardState(NULL);

	if (kstates[SDL_SCANCODE_RETURN] && kstates[SDL_SCANCODE_LALT]) {
		switchFullscreen();
	}
}

void updateSystem() {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) { 
		if( e.type == SDL_QUIT ) 
		{  
			returnToMenu();
		}
		else if (e.type == SDL_WINDOWEVENT) {
			checkWindowEvents(&e);
		}
	}

	checkFullscreen();
	
}

void setScreen(int tX, int tY, int tFramerate, int tIsVGA) {
	(void)tIsVGA;
	(void)tFramerate;
	if(!gData.mIsLoaded) initScreenDefault();
	gData.mScreenSizeX = gData.mDisplayedWindowSizeX = tX;
	gData.mScreenSizeY = gData.mDisplayedWindowSizeY = tY;
}

void setScreenSize(int tX, int tY) {
	if(!gData.mIsLoaded) initScreenDefault();

	gData.mScreenSizeX = gData.mDisplayedWindowSizeX = tX;
	gData.mScreenSizeY = gData.mDisplayedWindowSizeY = tY;
}

ScreenSize getScreenSize() {
	if(!gData.mIsLoaded) initScreenDefault();
	ScreenSize ret;
	ret.x = gData.mScreenSizeX;
	ret.y = gData.mScreenSizeY;
	return ret;
}

void setScreenFramerate(int tFramerate) {
	(void)tFramerate;
}

void setVGA() {
	
}

void returnToMenu() {
	exit(0);
}