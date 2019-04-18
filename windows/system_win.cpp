#include "prism/system.h"

#include <string.h>
#include <assert.h>

#include <stdlib.h>
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL/SDL_image.h>
#elif defined _WIN32


#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL_image.h>
#endif

#include "prism/log.h"
#include "prism/pvr.h"
#include "prism/geometry.h"
#include "prism/math.h"
#include "prism/wrapper.h"

void abortSystem(){
 	assert(0);
	exit(0);
}

// TODO: move to general
void recoverFromError()
{
	if (isUsingWrapper()) {
		recoverWrapperError();
	}
	else {
		abortSystem();
	}
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
SDL_GLContext gGLContext;

static void initScreenDefault() {
	gData.mIsLoaded = 1;
	gData.mScreenSizeX = gData.mDisplayedWindowSizeX = 640;
	gData.mScreenSizeY = gData.mDisplayedWindowSizeY = 480; 

	gData.mIsFullscreen = 0;
}

void setGameName(const char* tName) {
	strcpy(gData.mGameName, tName);
}

void updateGameName(const char * tName)
{
	strcpy(gData.mGameName, tName);
	SDL_SetWindowTitle(gSDLWindow, gData.mGameName);
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

extern void setDrawingScreenScale(double tScaleX, double tScaleY);

static void setWindowSize() {
	ScreenSize sz = getScreenSize();
	double scaleX = gData.mDisplayedWindowSizeX / (double)sz.x;
	double scaleY = gData.mDisplayedWindowSizeY / (double)sz.y;

	scaleX = fmin(scaleX, scaleY);
	scaleY = fmin(scaleX, scaleY);

	setDrawingScreenScale(scaleX, scaleY);
	SDL_SetWindowSize(gSDLWindow, (int)(scaleX * sz.x), (int)(scaleY * sz.y));
}

static void initOpenGL() {
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	gGLContext = SDL_GL_CreateContext(gSDLWindow);
}

static void initGlew() {
	glewExperimental = GL_TRUE;
	glewInit();
}

void initSystem() {

	setToProgramDirectory();
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	
	if (gData.mGameName[0] == '\0') {
		sprintf(gData.mGameName, "Unnamed libtari game port");
	}
	gSDLWindow = SDL_CreateWindow(gData.mGameName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	initOpenGL();

	initGlew();
}

void shutdownSystem() {
	SDL_GL_DeleteContext(gGLContext);
	
	SDL_DestroyWindow(gSDLWindow);

	IMG_Quit();
	SDL_Quit();
}

static void resizeWindow(SDL_Event* e) {
	setDisplayedScreenSize(e->window.data1, e->window.data2);
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
	gData.mScreenSizeX = tX;
	gData.mScreenSizeY = tY;
}

void setScreenSize(int tX, int tY) {
	if(!gData.mIsLoaded) initScreenDefault();

	gData.mScreenSizeX = tX;
	gData.mScreenSizeY = tY;
}

ScreenSize getScreenSize() {
	if(!gData.mIsLoaded) initScreenDefault();
	ScreenSize ret;
	ret.x = gData.mScreenSizeX;
	ret.y = gData.mScreenSizeY;
	return ret;
}

void setDisplayedScreenSize(int tX, int tY)
{
	gData.mDisplayedWindowSizeX = tX;
	gData.mDisplayedWindowSizeY = tY;

	setWindowSize();
}

void setScreenFramerate(int tFramerate) {
	(void)tFramerate;
}

void setVGA() {
	
}

void returnToMenu() {
	exit(0);
}

int isOnDreamcast()
{
	return 0;
}

int isOnWindows()
{
#ifdef __EMSCRIPTEN__
	return 0;
#else
	return 1;
#endif
}

int isOnWeb()
{
	return !isOnWindows();
}

uint64_t getSystemTicks() {
	return SDL_GetTicks();
}