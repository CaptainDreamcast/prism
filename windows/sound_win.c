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

int getVolume() {
	return gData.mVolume;
}

int getPanningValue() {
	return gData.mPanning;
}

void playTrack(int tTrack) {
	logWarning("Unable to play tracks on Windows.");
	// TODO: implement
}