#include "prism/system.h"

#include <string.h>
#include <assert.h>

#include <stdlib.h>
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL_image.h>
#elif defined _WIN32


#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glu.h>

#include <SDL_image.h>
#endif

#include "prism/log.h"
#include "prism/geometry.h"
#include "prism/math.h"
#include "prism/wrapper.h"
#include "prism/profiling.h"

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
} gPrismWindowsSystemData;

SDL_Window* gSDLWindow;
SDL_GLContext gGLContext;

static void initScreenDefault() {
	gPrismWindowsSystemData.mIsLoaded = 1;
	gPrismWindowsSystemData.mScreenSizeX = gPrismWindowsSystemData.mDisplayedWindowSizeX = 640;
	gPrismWindowsSystemData.mScreenSizeY = gPrismWindowsSystemData.mDisplayedWindowSizeY = 480; 

	gPrismWindowsSystemData.mIsFullscreen = 0;
}

void setGameName(const char* tName) {
	strcpy(gPrismWindowsSystemData.mGameName, tName);
}

void updateGameName(const char * tName)
{
	strcpy(gPrismWindowsSystemData.mGameName, tName);
	SDL_SetWindowTitle(gSDLWindow, gPrismWindowsSystemData.mGameName);
}

void setIcon(const char * tPath)
{
	SDL_Surface* icon = IMG_Load(tPath);
	SDL_SetWindowIcon(gSDLWindow, icon);
	SDL_FreeSurface(icon);
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

static void setWindowSize(int tX, int tY) {
	ScreenSize sz = getScreenSize();
	double scaleX = tX / (double)sz.x;
	double scaleY = tY / (double)sz.y;

	scaleX = fmin(scaleX, scaleY);
	scaleY = fmin(scaleX, scaleY);

	setDrawingScreenScale(scaleX, scaleY);
	gPrismWindowsSystemData.mDisplayedWindowSizeX = (int)(scaleX * sz.x);
	gPrismWindowsSystemData.mDisplayedWindowSizeY = (int)(scaleY * sz.y);
	SDL_SetWindowSize(gSDLWindow, gPrismWindowsSystemData.mDisplayedWindowSizeX, gPrismWindowsSystemData.mDisplayedWindowSizeY);
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
	
	if (gPrismWindowsSystemData.mGameName[0] == '\0') {
		sprintf(gPrismWindowsSystemData.mGameName, "Unnamed libtari game port");
	}
	gSDLWindow = SDL_CreateWindow(gPrismWindowsSystemData.mGameName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

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
	double scaleX = gPrismWindowsSystemData.mDisplayedWindowSizeX / (double)sz.x;
	double scaleY = gPrismWindowsSystemData.mDisplayedWindowSizeY / (double)sz.y;
	return vecScale3D(v, makePosition(1 / scaleX, 1 / scaleY, 1));
}

static void switchFullscreen() {
	if (!gPrismWindowsSystemData.mIsFullscreen) {
		SDL_SetWindowFullscreen(gSDLWindow, SDL_WINDOW_FULLSCREEN);
	}
	else {
		SDL_SetWindowFullscreen(gSDLWindow, 0);
	}

	gPrismWindowsSystemData.mIsFullscreen ^= 1;
}

static void checkFullscreen() {
	const Uint8* kstates = SDL_GetKeyboardState(NULL);

	if (kstates[SDL_SCANCODE_RETURN] && kstates[SDL_SCANCODE_LALT]) {
		switchFullscreen();
	}
}

extern void receiveCharacterInputFromSDL(const std::string& tText);

void updateSystem() {
	setProfilingSectionMarkerCurrentFunction();
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) { 
		switch (e.type) {
		case SDL_QUIT:
			returnToMenu();
			break;
		case SDL_WINDOWEVENT:
			checkWindowEvents(&e);
			break;
		case SDL_TEXTINPUT:
			receiveCharacterInputFromSDL(e.text.text);
			break;
		default:
			break;
		}
	}

	checkFullscreen();
	
}

void setScreen(int tX, int tY, int tFramerate, int tIsVGA) {
	(void)tIsVGA;
	(void)tFramerate;
	if(!gPrismWindowsSystemData.mIsLoaded) initScreenDefault();
	gPrismWindowsSystemData.mScreenSizeX = tX;
	gPrismWindowsSystemData.mScreenSizeY = tY;
}

void setScreenSize(int tX, int tY) {
	if(!gPrismWindowsSystemData.mIsLoaded) initScreenDefault();

	gPrismWindowsSystemData.mScreenSizeX = tX;
	gPrismWindowsSystemData.mScreenSizeY = tY;
}

ScreenSize getScreenSize() {
	if(!gPrismWindowsSystemData.mIsLoaded) initScreenDefault();
	ScreenSize ret;
	ret.x = gPrismWindowsSystemData.mScreenSizeX;
	ret.y = gPrismWindowsSystemData.mScreenSizeY;
	return ret;
}

ScreenSize getDisplayedScreenSize()
{
	if (!gPrismWindowsSystemData.mIsLoaded) initScreenDefault();
	ScreenSize ret;
	ret.x = gPrismWindowsSystemData.mDisplayedWindowSizeX;
	ret.y = gPrismWindowsSystemData.mDisplayedWindowSizeY;
	return ret;
}

void setDisplayedScreenSize(int tX, int tY)
{
	setWindowSize(tX, tY);
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