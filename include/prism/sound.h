#pragma once

#include <stdint.h>

#include "actorhandler.h"

namespace prism {

void initSound();
void shutdownSound();
void updateSound();

double getVolume();
void setVolume(double tVolume);
double getPanningValue();
void setPanningValue(double tPanning);

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
void pauseMusic();
void resumeMusic();
void crossFadeMusicLayer(const char* tNewPath, bool tIsLooping);

ActorBlueprint getMicrophoneHandlerActorBlueprint();
double getMicrophoneVolume();

#ifdef _WIN32
void imguiSoundHardware();
#endif

}