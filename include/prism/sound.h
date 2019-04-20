#pragma once

#include <stdint.h>

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

void streamMusicFile(const char* tPath);
void streamMusicFileOnce(const char* tPath);
void stopStreamingMusicFile();
uint64_t getStreamingSoundTimeElapsedInMilliseconds();
int isPlayingStreamingMusic();
void stopMusic();

ActorBlueprint getMicrophoneHandlerActorBlueprint();
double getMicrophoneVolume();
