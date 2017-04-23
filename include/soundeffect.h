#ifndef TARI_SOUNDEFFECT_H
#define TARI_SOUNDEFFECT_H

#include "common/header.h"

fup void initSoundEffects();
fup void setupSoundEffectHandler();
fup void shutdownSoundEffectHandler();

fup int loadSoundEffect(char* tPath);
fup void unloadSoundEffect(int tID);
fup void playSoundEffect(int tID);

fup double getSoundEffectVolume();
fup void setSoundEffectVolume(double tVolume);

#endif
