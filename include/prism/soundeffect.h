#pragma once

#include "file.h"

typedef struct {
	int mAmount;
	int* mSoundEffects;
} SoundEffectCollection;

void initSoundEffects();
void setupSoundEffectHandler();
void shutdownSoundEffectHandler();

void setSoundEffectCompression(int tIsEnabled);

int loadSoundEffect(char* tPath);
int loadSoundEffectFromBuffer(Buffer tBuffer);
void unloadSoundEffect(int tID);
int playSoundEffect(int tID);
void stopSoundEffect(int tSFX);
SoundEffectCollection loadConsecutiveSoundEffectsToCollection(char* tPath, int tAmount);
void loadConsecutiveSoundEffects(int* tDst, char* tPath, int tAmount);
int playRandomSoundEffectFromCollection(SoundEffectCollection tCollection);

double getSoundEffectVolume();
void setSoundEffectVolume(double tVolume);

