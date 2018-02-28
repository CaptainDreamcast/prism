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


MugenSounds loadMugenSoundFile(char* tPath);
void playMugenSound(MugenSounds* tSounds, int tGroup, int tSample);