#include "../include/sound.h"

#include <dc/sound/sound.h>

static struct {

	double mVolume;
	double mPanning;

} gData;

void initSound() {
	gData.mVolume = 1;
	gData.mPanning = 0;
	snd_init();
}

void shutdownSound(){
	
}

double getVolume() {
	return gData.mVolume;
}

void setVolume(double tVolume) {
	gData.mVolume = tVolume;
}

double getPanningValue() {
	return gData.mPanning;
}



