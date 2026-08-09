#pragma once

#include <string>
#include "file.h"

namespace prism {

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
int playSoundEffectChannel(int tID, int tChannel, float tVolume, float tFreqMul = 1.0, int tIsLooping = 0);
int playSoundEffectFile(const std::string& tPath);
void stopSoundEffect(int tChannel);
void stopAllSoundEffects();
void panSoundEffect(int tChannel, float tPanning);
int isSoundEffectPlayingOnChannel(int tChannel);
SoundEffectCollection loadConsecutiveSoundEffectsToCollection(const char* tPath, int tAmount);
void loadConsecutiveSoundEffects(int* tDst, const char* tPath, int tAmount);
int playRandomSoundEffectFromCollection(const SoundEffectCollection& tCollection);

float getSoundEffectVolume();
void setSoundEffectVolume(float tVolume);

#ifdef _WIN32
void imguiSoundEffectsHardware();
#endif

}