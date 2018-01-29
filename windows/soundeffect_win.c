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

static int addChunkToSoundEffectHandler(Mix_Chunk* tChunk) {
	SoundEffectEntry* e = allocMemory(sizeof(SoundEffectEntry));
	e->mChunk = tChunk;
	return list_push_back_owned(&gData.mAllocatedChunks, e);
}

int loadSoundEffect(char* tPath) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);
	Mix_Chunk* chunk = Mix_LoadWAV(fullPath);
	return addChunkToSoundEffectHandler(chunk);
}

static int gDummy;

int loadSoundEffectFromBuffer(Buffer tBuffer) {
	SDL_RWops* rwOps = SDL_RWFromConstMem(tBuffer.mData, tBuffer.mLength);
	Mix_Chunk* chunk = Mix_LoadWAV_RW(rwOps, 0);
	return addChunkToSoundEffectHandler(chunk);
}

void unloadSoundEffect(int tID) {
	SoundEffectEntry* e = list_get(&gData.mAllocatedChunks, tID);
	unloadSoundEffectEntry(e);
	list_remove(&gData.mAllocatedChunks, tID);
}

int playSoundEffect(int tID) {
	SoundEffectEntry* e = list_get(&gData.mAllocatedChunks, tID);
	return Mix_PlayChannel(-1, e->mChunk, 0);
}

void stopSoundEffect(int tSFX) {
	Mix_HaltChannel(tSFX);
}

double getSoundEffectVolume() {
	return gData.mVolume;
}

void setSoundEffectVolume(double tVolume) {
	gData.mVolume = tVolume;
	// TODO
}
