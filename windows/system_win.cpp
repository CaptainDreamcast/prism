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
#include "prism/input.h"

void abortSystem(){
 	assert(0);
	exit(0);
}

static struct {
	
	int mIsLoaded;
	int mIsExitDisabled;
	
	int mScreenSizeX;
	int mScreenSizeY;
	
	int mDisplayedWindowSizeX;
	int mDisplayedWindowSizeY;
		
	char mGameName[100];
} gPrismWindowsSystemData;

SDL_Window* gSDLWindow;
SDL_GLContext gGLContext;

static void initScreenDefault() {
	gPrismWindowsSystemData.mIsLoaded = 1;
	gPrismWindowsSystemData.mIsExitDisabled = 0;
	gPrismWindowsSystemData.mScreenSizeX = gPrismWindowsSystemData.mDisplayedWindowSizeX = 640;
	gPrismWindowsSystemData.mScreenSizeY = gPrismWindowsSystemData.mDisplayedWindowSizeY = 480; 
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
#define Rectangle Rectangle2
#include <windows.h>
#include <direct.h>
#undef Rectangle
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

Vector3D correctSDLWindowPosition(const Vector3D& v) {
	ScreenSize sz = getScreenSize();
	double scaleX = gPrismWindowsSystemData.mDisplayedWindowSizeX / (double)sz.x;
	double scaleY = gPrismWindowsSystemData.mDisplayedWindowSizeY / (double)sz.y;
	return vecScale3D(v, Vector3D(1 / scaleX, 1 / scaleY, 1));
}

static void switchFullscreen() {
	const auto flags = SDL_GetWindowFlags(gSDLWindow);
	if (!(flags & SDL_WINDOW_FULLSCREEN)) {
		if (isOnWeb()) {
			SDL_SetWindowFullscreen(gSDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP); // does not break window resizing, unlike the other one, probably due to https://github.com/emscripten-ports/SDL2/issues/8
		}
		else {
			SDL_SetWindowFullscreen(gSDLWindow, SDL_WINDOW_FULLSCREEN);
		}
	}
	else {
		SDL_SetWindowFullscreen(gSDLWindow, 0);
		if (isOnWeb()) {
			setWindowSize(640, 480); // force real window size after fullscreen switch until (https://github.com/emscripten-ports/SDL2/issues/8) resolved
		}
	}
}

static void checkFullscreen() {
	const auto webInput = isOnWeb() && hasPressedKeyboardKeyFlank(KEYBOARD_F8_PRISM);
	const auto nonWebInput = !isOnWeb() && hasPressedKeyboardMultipleKeyFlank(2, KEYBOARD_ALT_LEFT_PRISM, KEYBOARD_RETURN_PRISM);
	if (webInput || nonWebInput) {
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
	if (!gPrismWindowsSystemData.mIsExitDisabled)
	{
		exit(0);
	}
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

int isOnVita()
{
	return 0;
}

uint64_t getSystemTicks() {
	return SDL_GetTicks();
}

void setSystemExitDisabled(int tIsExitDisabled)
{
	gPrismWindowsSystemData.mIsExitDisabled = tIsExitDisabled;
}