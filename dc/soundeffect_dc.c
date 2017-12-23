#include "tari/soundeffect.h"

#include <kos.h>

#include "tari/file.h"
#include "tari/sound.h"

void initSoundEffects() {

}

void setupSoundEffectHandler() {
	
}

void shutdownSoundEffectHandler() {
	snd_sfx_unload_all();
}

int loadSoundEffect(char* tPath) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);
	return snd_sfx_load(fullPath);
}

void unloadSoundEffect(int tID) {
	snd_sfx_unload(tID);
}

int playSoundEffect(int tID) {
	return snd_sfx_play(tID, getVolume(), getPanningValue());
}

void stopSoundEffect(int tSFX) {
	snd_sfx_stop(tSFX);
}

void setSoundEffectVolume(double tVolume) {
	(void) tVolume;
}
