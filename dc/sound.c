#include "../include/sound.h"

#include <dc/sound/sound.h>

static struct {

	int mVolume;
	int mPanning;

} gData;

void initSound() {
	gData.mVolume = 255;
	gData.mPanning = 128;
	snd_init();
}

void shutdownSound(){
	
}

int getVolume() {
	return gData.mVolume;
}

int getPanningValue() {
	return gData.mPanning;
}
