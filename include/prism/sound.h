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

void streamMusicFile(const char* tPath, int tIsForcingSynchronously = 0);
void streamMusicFileOnce(const char* tPath, int tIsForcingSynchronously = 0);
void stopStreamingMusicFile();
uint64_t getStreamingSoundTimeElapsedInMilliseconds();
int isPlayingStreamingMusic();
void stopMusic();
void pauseMusic();
void resumeMusic();

ActorBlueprint getMicrophoneHandlerActorBlueprint();
double getMicrophoneVolume();
