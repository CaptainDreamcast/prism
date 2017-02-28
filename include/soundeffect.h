#ifndef TARI_SOUNDEFFECT_H
#define TARI_SOUNDEFFECT_H

void setupSoundEffectHandler();
void shutdownSoundEffectHandler();

int loadSoundEffect(char* tPath);
void playSoundEffect(int tID);

#endif
