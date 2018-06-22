#pragma once

#include "actorhandler.h"

void initSound();
void shutdownSound();

double getVolume();
void setVolume(double tVolume);
double getPanningValue();

void playTrack(int tTrack);
void stopTrack();
void pauseTrack();
void resumeTrack();
void playTrackOnce(int tTrack);

void streamMusicFile(char* tPath);

ActorBlueprint getMicrophoneHandlerActorBlueprint();
double getMicrophoneVolume();
