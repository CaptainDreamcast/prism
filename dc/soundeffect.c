#include "../include/soundeffect.h"

#include <kos.h>

#include "../include/file.h"
#include "../include/sound.h"

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

void playSoundEffect(int tID) {
	snd_sfx_play(tID, getVolume(), getPanningValue());
}

