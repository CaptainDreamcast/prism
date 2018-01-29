#ifndef TARI_SOUNDEFFECT_H
#define TARI_SOUNDEFFECT_H

#include "common/header.h"

#include "file.h"

typedef struct {
	int mAmount;
	int* mSoundEffects;
} SoundEffectCollection;

fup void initSoundEffects();
fup void setupSoundEffectHandler();
fup void shutdownSoundEffectHandler();

fup int loadSoundEffect(char* tPath);
int loadSoundEffectFromBuffer(Buffer tBuffer);
fup void unloadSoundEffect(int tID);
fup int playSoundEffect(int tID);
fup void stopSoundEffect(int tSFX);
fup SoundEffectCollection loadConsecutiveSoundEffectsToCollection(char* tPath, int tAmount);
fup void loadConsecutiveSoundEffects(int* tDst, char* tPath, int tAmount);
fup int playRandomSoundEffectFromCollection(SoundEffectCollection tCollection);

fup double getSoundEffectVolume();
fup void setSoundEffectVolume(double tVolume);

#endif
