#pragma once

#include "common/header.h"

#include <tari/actorhandler.h>

fup void initSound();
fup void shutdownSound();

fup double getVolume();
fup void setVolume(double tVolume);
fup double getPanningValue();

fup void playTrack(int tTrack);
fup void stopTrack();
fup void pauseTrack();
fup void resumeTrack();

fup ActorBlueprint getMicrophoneHandlerActorBlueprint();
double getMicrophoneVolume();
