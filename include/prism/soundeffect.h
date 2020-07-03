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

int loadSoundEffect(const char* tPath);
int loadSoundEffectFromBuffer(const Buffer& tBuffer);
void unloadSoundEffect(int tID);
int playSoundEffect(int tID);
int playSoundEffectChannel(int tID, int tChannel, double tVolume, double tFreqMul = 1.0, int tIsLooping = 0);
void stopSoundEffect(int tChannel);
void stopAllSoundEffects();
void panSoundEffect(int tChannel, double tPanning);
int isSoundEffectPlayingOnChannel(int tChannel);
SoundEffectCollection loadConsecutiveSoundEffectsToCollection(const char* tPath, int tAmount);
void loadConsecutiveSoundEffects(int* tDst, const char* tPath, int tAmount);
int playRandomSoundEffectFromCollection(const SoundEffectCollection& tCollection);

double getSoundEffectVolume();
void setSoundEffectVolume(double tVolume);
