#include "tari/system.h"

#include <kos.h>

#include "tari/log.h"
#include "tari/pvr.h"

void abortSystem(){
	arch_exit();
}	

void returnToMenu() {
	arch_menu();
}

static struct {

	int mIsLoaded;
	
	int mScreenSizeX;
	int mScreenSizeY;

	int mFramerate;
	int mIsVGA;

} gSystem;

void initSystem(){}

void shutdownSystem(){}

void updateSystem() {}

static void initScreenDefault() {
	gSystem.mIsLoaded = 1;
	gSystem.mScreenSizeX = 640;
	gSystem.mScreenSizeY = 480;
	gSystem.mFramerate = 60;
	gSystem.mIsVGA = 0;	
}

 // TODO: enable video mode changing on the fly
static void setVideoModeInternal() {

	if(gSystem.mScreenSizeX == 640 && gSystem.mScreenSizeY == 480 && gSystem.mIsVGA) {
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	} else if(gSystem.mScreenSizeX == 640 && gSystem.mScreenSizeY == 480 && gSystem.mFramerate == 50) {
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
	} else if(gSystem.mScreenSizeX == 640 && gSystem.mScreenSizeY == 480 && gSystem.mFramerate == 60) {
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	} else if(gSystem.mScreenSizeX == 320 && gSystem.mScreenSizeY == 240 && gSystem.mIsVGA) {
		vid_set_mode(DM_320x240_VGA, PM_RGB565);
	} else if(gSystem.mScreenSizeX == 320 && gSystem.mScreenSizeY == 240 && gSystem.mFramerate == 50) {
		vid_set_mode(DM_320x240_PAL, PM_RGB565);
	} else if(gSystem.mScreenSizeX == 320 && gSystem.mScreenSizeY == 240 && gSystem.mFramerate == 60) {
		vid_set_mode(DM_320x240_NTSC, PM_RGB565);
	} else {
		logError("Unrecognized video format.");
		logErrorInteger(gSystem.mScreenSizeX);
		logErrorInteger(gSystem.mScreenSizeY);
		logErrorInteger(gSystem.mFramerate);
		logErrorInteger(gSystem.mIsVGA);
		abortSystem();
	}

}

void setScreen(int tX, int tY, int tFramerate, int tIsVGA) {
	if(!gSystem.mIsLoaded) initScreenDefault();
	gSystem.mScreenSizeX = tX;
	gSystem.mScreenSizeY = tY;
	gSystem.mFramerate = tFramerate;
	gSystem.mIsVGA = tIsVGA;
	
	setVideoModeInternal();
	
}

void setScreenSize(int tX, int tY) {
	if(!gSystem.mIsLoaded) initScreenDefault();

	gSystem.mScreenSizeX = tX;
	gSystem.mScreenSizeY = tY;
	
	setVideoModeInternal();
}

ScreenSize getScreenSize() {
	if(!gSystem.mIsLoaded) initScreenDefault();
	ScreenSize ret;
	ret.x = gSystem.mScreenSizeX;
	ret.y = gSystem.mScreenSizeY;
	return ret;
}

void setScreenFramerate(int tFramerate) {
	if(!gSystem.mIsLoaded) initScreenDefault();
	gSystem.mFramerate = tFramerate;

	setVideoModeInternal();
}

void setVGA() {
	if(!gSystem.mIsLoaded) initScreenDefault();
	gSystem.mFramerate = 60;
	gSystem.mIsVGA = 1;
	setVideoModeInternal();
}


void setGameName(char* tName) {
	(void) tName;
}

