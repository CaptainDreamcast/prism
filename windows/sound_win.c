#include "../include/sound.h"

#include <SDL_mixer.h>

#include "../include/log.h"

static struct {

	int mVolume;
	int mPanning;

} gData;

void initSound() {
	gData.mVolume = 255;
	gData.mPanning = 128;
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
}

void shutdownSound() {
	Mix_CloseAudio();
}

double getVolume() {
	return gData.mVolume / 255.0;
}

void setVolume(double tVolume) {
	gData.mVolume = (int)(tVolume * 255);
}

double getPanningValue() {
	return (gData.mPanning / 128.0) - 1.0;
}

void playTrack(int tTrack) {
	(void)tTrack;
	logWarning("Unable to play tracks on Windows.");
	// TODO: implement
}

