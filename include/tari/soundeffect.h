#ifndef TARI_SOUNDEFFECT_H
#define TARI_SOUNDEFFECT_H

#include "common/header.h"

typedef struct {
	int mAmount;
	int* mSoundEffects;
} SoundEffectCollection;

fup void initSoundEffects();
fup void setupSoundEffectHandler();
fup void shutdownSoundEffectHandler();

fup int loadSoundEffect(char* tPath);
fup void unloadSoundEffect(int tID);
fup void playSoundEffect(int tID);
fup SoundEffectCollection loadConsecutiveSoundEffectsToCollection(char* tPath, int tAmount);
fup void loadConsecutiveSoundEffects(int* tDst, char* tPath, int tAmount);
fup void playRandomSoundEffectFromCollection(SoundEffectCollection tCollection);

fup double getSoundEffectVolume();
fup void setSoundEffectVolume(double tVolume);

#endif
