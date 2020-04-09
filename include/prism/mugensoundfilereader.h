#pragma once

#include "datastructures.h"

typedef struct {
	int mSoundEffectID;
} MugenSoundSample;

typedef struct {
	IntMap mSamples;
} MugenSoundGroup;

typedef struct {
	IntMap mGroups;
} MugenSounds;


MugenSounds loadMugenSoundFile(const char* tPath);
void unloadMugenSoundFile(MugenSounds* tSounds);
MugenSounds createEmptyMugenSoundFile();
int playMugenSound(MugenSounds* tSounds, int tGroup, int tSample);
int playMugenSoundAdvanced(MugenSounds* tSounds, int tGroup, int tSample, double tVolume, int tChannel, double tFrequencyMultiplier, int tIsLooping, double tPanning);
int tryPlayMugenSound(MugenSounds* tSounds, int tGroup, int tSample);
int tryPlayMugenSoundAdvanced(MugenSounds* tSounds, int tGroup, int tSample, double tVolume, int tChannel = -1, double tFrequencyMultiplier = 1.0, int tIsLooping = 0, double tPanning = 0.0);
int hasMugenSound(MugenSounds* tSounds, int tGroup, int tSample);