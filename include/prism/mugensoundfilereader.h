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
int tryPlayMugenSound(MugenSounds* tSounds, int tGroup, int tSample);
int hasMugenSound(MugenSounds* tSounds, int tGroup, int tSample);