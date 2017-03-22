#include "../include/system.h"

#include <kos.h>

#include "../include/log.h"
#include "../include/pvr.h"

void abortSystem(){
	arch_exit();
}	

static struct {

	int mIsLoaded;
	
	int mScreenSizeX;
	int mScreenSizeY;

	int mFramerate;
	int mIsVGA;

} gData;


static void initScreenDefault() {
	gData.mIsLoaded = 1;
	gData.mScreenSizeX = 640;
	gData.mScreenSizeY = 480;
	gData.mFramerate = 60;
	gData.mIsVGA = 0;	
}

 // TODO: enable video mode changing on the fly
static void setVideoModeInternal() {

	if(gData.mScreenSizeX == 640 && gData.mScreenSizeY == 480 && gData.mIsVGA) {
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	} else if(gData.mScreenSizeX == 640 && gData.mScreenSizeY == 480 && gData.mFramerate == 50) {
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
	} else if(gData.mScreenSizeX == 640 && gData.mScreenSizeY == 480 && gData.mFramerate == 60) {
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	} else if(gData.mScreenSizeX == 320 && gData.mScreenSizeY == 240 && gData.mIsVGA) {
		vid_set_mode(DM_320x240_VGA, PM_RGB565);
	} else if(gData.mScreenSizeX == 320 && gData.mScreenSizeY == 240 && gData.mFramerate == 50) {
		vid_set_mode(DM_320x240_PAL, PM_RGB565);
	} else if(gData.mScreenSizeX == 320 && gData.mScreenSizeY == 240 && gData.mFramerate == 60) {
		vid_set_mode(DM_320x240_NTSC, PM_RGB565);
	} else {
		logError("Unrecognized video format.");
		logErrorInteger(gData.mScreenSizeX);
		logErrorInteger(gData.mScreenSizeY);
		logErrorInteger(gData.mFramerate);
		logErrorInteger(gData.mIsVGA);
		abortSystem();
	}

}

void setScreen(int tX, int tY, int tFramerate, int tIsVGA) {
	if(!gData.mIsLoaded) initScreenDefault();
	gData.mScreenSizeX = tX;
	gData.mScreenSizeY = tY;
	gData.mFramerate = tFramerate;
	gData.mIsVGA = tIsVGA;
	
	setVideoModeInternal();
	
}

void setScreenSize(int tX, int tY) {
	if(!gData.mIsLoaded) initScreenDefault();

	gData.mScreenSizeX = tX;
	gData.mScreenSizeY = tY;
	
	setVideoModeInternal();
}

ScreenSize getScreenSize() {
	if(!gData.mIsLoaded) initScreenDefault();
	ScreenSize ret;
	ret.x = gData.mScreenSizeX;
	ret.y = gData.mScreenSizeY;
	return ret;
}

void setScreenFramerate(int tFramerate) {
	if(!gData.mIsLoaded) initScreenDefault();
	gData.mFramerate = tFramerate;

	setVideoModeInternal();
}

void setVGA() {
	if(!gData.mIsLoaded) initScreenDefault();
	gData.mFramerate = 60;
	gData.mIsVGA = 1;
	setVideoModeInternal();
}
