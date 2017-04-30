#ifndef TARI_SOUND_H
#define TARI_SOUND_H

#include "common/header.h"

fup void initSound();
fup void shutdownSound();

fup double getVolume();
fup void setVolume(double tVolume);
fup double getPanningValue();

fup void playTrack(int tTrack);

#endif 
