#include "../include/soundeffect.h"

#include <SDL_mixer.h>

#include "../include/file.h"
#include "../include/sound.h"
#include "../include/datastructures.h"
#include "../include/memoryhandler.h"

typedef struct {

	Mix_Chunk* mChunk;

} SoundEffectEntry;

static struct {
	List mAllocatedChunks;
} gData;

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

void playSoundEffect(int tID) {
	SoundEffectEntry* e = list_get(&gData.mAllocatedChunks, tID);
	Mix_PlayChannel(-1, e->mChunk, 0);
}


