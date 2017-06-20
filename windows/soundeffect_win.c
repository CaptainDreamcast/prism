#include "tari/soundeffect.h"

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_mixer.h>
#elif defined _WIN32
#include <SDL_mixer.h>
#endif


#include "tari/file.h"
#include "tari/sound.h"
#include "tari/datastructures.h"
#include "tari/memoryhandler.h"

typedef struct {

	Mix_Chunk* mChunk;

} SoundEffectEntry;

static struct {
	double mVolume;
	List mAllocatedChunks;
} gData;

void initSoundEffects() {
	setSoundEffectVolume(1);
}

void setupSoundEffectHandler() {
	gData.mAllocatedChunks = new_list();
	
}

static void unloadSoundEffectEntry(SoundEffectEntry* e) {
	Mix_FreeChunk(e->mChunk);
}

static int unloadSingleSoundEffect(void* tCaller, void* tData) {
	(void)tCaller;
	SoundEffectEntry* e = tData;
	unloadSoundEffectEntry(e);
	return 1;
}


void shutdownSoundEffectHandler() {
	list_remove_predicate(&gData.mAllocatedChunks, unloadSingleSoundEffect, NULL);
}

int loadSoundEffect(char* tPath) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);
	Mix_Chunk* chunk = Mix_LoadWAV(fullPath);

	SoundEffectEntry* e = allocMemory(sizeof(SoundEffectEntry));
	e->mChunk = chunk;
	
	return list_push_back_owned(&gData.mAllocatedChunks, e);
}

void unloadSoundEffect(int tID) {
	SoundEffectEntry* e = list_get(&gData.mAllocatedChunks, tID);
	unloadSoundEffectEntry(e);
	list_remove(&gData.mAllocatedChunks, tID);
}

int playSoundEffect(int tID) {
	SoundEffectEntry* e = list_get(&gData.mAllocatedChunks, tID);
	return 0;
	return Mix_PlayChannel(-1, e->mChunk, 0);
}

void stopSoundEffect(int tSFX) {
	return;
	Mix_HaltChannel(tSFX);
}

double getSoundEffectVolume() {
	return gData.mVolume;
}

void setSoundEffectVolume(double tVolume) {
	gData.mVolume = tVolume;
	// TODO
}
