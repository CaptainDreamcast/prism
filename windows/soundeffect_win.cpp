#include "prism/soundeffect.h"

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#elif defined _WIN32
#include <SDL_mixer.h>
#endif

#include <algorithm>

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
	double mVolume;
	map<int, SoundEffectEntry> mAllocatedChunks;
	map<int, Mix_Chunk*> mChunks;
} gSoundEffectData;

void initSoundEffects() {
	gSoundEffectData.mVolume = 20;
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

void setSoundEffectCompression(int /*tIsEnabled*/) {} // no need for compression in web/windows

static int addBufferToSoundEffectHandler(Buffer tBuffer) {
	SoundEffectEntry e;
	e.mBuffer = tBuffer;
	return stl_int_map_push_back(gSoundEffectData.mAllocatedChunks, e);
}

int loadSoundEffect(const char* tPath) {
	Buffer b = fileToBuffer(tPath);
	return addBufferToSoundEffectHandler(b);
}

static int gDummy;

int loadSoundEffectFromBuffer(const Buffer& tBuffer) {

	Buffer ownedBuffer = copyBuffer(tBuffer);
	return addBufferToSoundEffectHandler(ownedBuffer);
}

void unloadSoundEffect(int tID) {
	SoundEffectEntry* e = &gSoundEffectData.mAllocatedChunks[tID];
	unloadSoundEffectEntry(e);
	gSoundEffectData.mAllocatedChunks.erase(tID);
}

static void tryEraseChannelChunk(int tChannel) {
	setProfilingSectionMarkerCurrentFunction();
	if (stl_map_contains(gSoundEffectData.mChunks, tChannel)) {
		Mix_FreeChunk(gSoundEffectData.mChunks[tChannel]);
		gSoundEffectData.mChunks.erase(tChannel);
	}
}

int playSoundEffect(int tID) {
	setProfilingSectionMarkerCurrentFunction();
	return playSoundEffectChannel(tID, -1, getSoundEffectVolume());
}

static int parseVolume(double tVolume) {
	return (int)(tVolume * 128);
}

int playSoundEffectChannel(int tID, int tChannel, double tVolume, double /*tFreqMul*/, int tIsLooping)
{
	setProfilingSectionMarkerCurrentFunction();
	SoundEffectEntry* e = &gSoundEffectData.mAllocatedChunks[tID];
	SDL_RWops* rwOps = SDL_RWFromConstMem(e->mBuffer.mData, e->mBuffer.mLength);
	Mix_Chunk* chunk = Mix_LoadWAV_RW(rwOps, 0);
	int channel = Mix_PlayChannel(tChannel, chunk, tIsLooping);
	Mix_Volume(channel, parseVolume(tVolume));
	tryEraseChannelChunk(channel);

	gSoundEffectData.mChunks[channel] = chunk;
	return channel;
}

void stopSoundEffect(int tChannel) {
	setProfilingSectionMarkerCurrentFunction();
	Mix_HaltChannel(tChannel);
	tryEraseChannelChunk(tChannel);
}

static void stopSingleSoundEffectCB(int tChannel, Mix_Chunk*& /*tChunk*/) {
	setProfilingSectionMarkerCurrentFunction();
	Mix_HaltChannel(tChannel);
	tryEraseChannelChunk(tChannel);
}

void stopAllSoundEffects() {
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gSoundEffectData.mChunks, stopSingleSoundEffectCB);
}

void panSoundEffect(int tChannel, double tPanning)
{
	setProfilingSectionMarkerCurrentFunction();
	const uint8_t right = uint8_t(std::min(std::max(tPanning, 0.0), 1.0) * 255);
	Mix_SetPanning(tChannel, 255 - right, right);
}

int isSoundEffectPlayingOnChannel(int tChannel) {
	setProfilingSectionMarkerCurrentFunction();
	return Mix_Playing(tChannel);
}

double getSoundEffectVolume() {
	return gSoundEffectData.mVolume;
}

void setSoundEffectVolume(double tVolume) {
	setProfilingSectionMarkerCurrentFunction();
	gSoundEffectData.mVolume = tVolume;
	Mix_Volume(-1, parseVolume(gSoundEffectData.mVolume));
}
