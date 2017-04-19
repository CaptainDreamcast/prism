#ifndef TARI_SOUNDEFFECT_H
#define TARI_SOUNDEFFECT_H

#include "common/header.h"

fup void setupSoundEffectHandler();
fup void shutdownSoundEffectHandler();

fup int loadSoundEffect(char* tPath);
fup void unloadSoundEffect(int tID);
fup void playSoundEffect(int tID);

#endif
