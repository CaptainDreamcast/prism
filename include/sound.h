#ifndef TARI_SOUND_H
#define TARI_SOUND_H

#include "common/header.h"

fup void initSound();
fup void shutdownSound();

fup int getVolume();
fup int getPanningValue();

fup void playTrack(int tTrack);

#endif 
