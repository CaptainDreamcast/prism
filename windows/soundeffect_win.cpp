#include "prism/soundeffect.h"

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_mixer.h>
#elif defined _WIN32
#include <SDL_mixer.h>
#endif


#include "prism/file.h"
#include "prism/sound.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/stlutil.h"

using namespace std;

typedef struct {
	Buffer mBuffer;
} SoundEffectEntry;

static struct {
	int mVolume;
	map<int, SoundEffectEntry> mAllocatedChunks;
	map<int, Mix_Chunk*> mChunks;
} gSoundEffectData;

void initSoundEffects() {
	
}

void setupSoundEffectHandler() {
	gSoundEffectData.mAllocatedChunks.clear();
	gSoundEffectData.mChunks.clear();
}

static void unloadSoundEffectEntry(SoundEffectEntry* e) {
	freeBuffer(e->mBuffer);
}

static int unloadSingleSoundEffect(void* tCaller, SoundEffectEntry& tData) {
	(void)tCaller;
	SoundEffectEntry* e = &tData;
	unloadSoundEffectEntry(e);
	return 1;
}

static int unloadSingleChunkEntry(void* tCaller, Mix_Chunk*& tData) {
	(void)tCaller;
	Mix_FreeChunk(tData);
	return 1;
}

void shutdownSoundEffectHandler() {
	stl_int_map_remove_predicate(gSoundEffectData.mAllocatedChunks, unloadSingleSoundEffect);
	gSoundEffectData.mAllocatedChunks.clear();

	stl_int_map_remove_predicate(gSoundEffectData.mChunks, unloadSingleChunkEntry);
	gSoundEffectData.mChunks.clear();
}

static int addBufferToSoundEffectHandler(Buffer tBuffer) {
	SoundEffectEntry e;
	e.mBuffer = tBuffer;
	return stl_int_map_push_back(gSoundEffectData.mAllocatedChunks, e);
}

int loadSoundEffect(char* tPath) {
	Buffer b = fileToBuffer(tPath);
	return addBufferToSoundEffectHandler(b);
}

static int gDummy;

int loadSoundEffectFromBuffer(Buffer tBuffer) {

	Buffer ownedBuffer = copyBuffer(tBuffer);
	return addBufferToSoundEffectHandler(ownedBuffer);
}

void unloadSoundEffect(int tID) {
	SoundEffectEntry* e = &gSoundEffectData.mAllocatedChunks[tID];
	unloadSoundEffectEntry(e);
	gSoundEffectData.mAllocatedChunks.erase(tID);
}

static void tryEraseChannelChunk(int tChannel) {
	if (stl_map_contains(gSoundEffectData.mChunks, tChannel)) {
		Mix_FreeChunk(gSoundEffectData.mChunks[tChannel]);
		gSoundEffectData.mChunks.erase(tChannel);
	}
}

int playSoundEffect(int tID) {
	SoundEffectEntry* e = &gSoundEffectData.mAllocatedChunks[tID];
	SDL_RWops* rwOps = SDL_RWFromConstMem(e->mBuffer.mData, e->mBuffer.mLength);
	Mix_Chunk* chunk = Mix_LoadWAV_RW(rwOps, 0);
	printf(Mix_GetError());
	int channel = Mix_PlayChannel(-1, chunk, 0);
	tryEraseChannelChunk(channel);

	gSoundEffectData.mChunks[channel] = chunk;
	return channel;
}

void stopSoundEffect(int tSFX) {
	Mix_HaltChannel(tSFX);
	tryEraseChannelChunk(tSFX);
}

double getSoundEffectVolume() {
	return gSoundEffectData.mVolume;
}

void setSoundEffectVolume(double tVolume) {
	gSoundEffectData.mVolume = (int)(tVolume * 128);
	Mix_Volume(-1, gSoundEffectData.mVolume);
}
