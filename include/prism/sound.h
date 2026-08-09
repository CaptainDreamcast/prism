#pragma once

#include <stdint.h>

#include "actorhandler.h"

namespace prism {

void initSound();
void shutdownSound();
void updateSound();

float getVolume();
void setVolume(float tVolume);
float getPanningValue();
void setPanningValue(float tPanning);

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
float getMicrophoneVolume();

#ifdef _WIN32
void imguiSoundHardware();
#endif

}