#include "prism/soundeffect.h"

extern "C"
{
#include <DrakonSound/DrakonSound.h>
}

#include <algorithm>

#include "prism/file.h"
#include "prism/sound.h"
#include "prism/datastructures.h"
#include "prism/stlutil.h"
#include "prism/log.h"

using namespace std;

typedef struct {
	Buffer mBuffer;
} SoundEffectEntry;

static struct {
	double mVolume;
	map<int, SoundEffectEntry> mAllocatedChunks;
	map<int, DrakonAudioHandler> mChunks;
} gSoundEffectData;

void initSoundEffects() {
	gSoundEffectData.mVolume = 20;
}

void setupSoundEffectHandler() {
	return;
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

static int unloadSingleChunkEntry(void* tCaller, DrakonAudioHandler& tData) {
	(void)tCaller;
	DrakonStopAudio(&tData);
	DrakonTerminateAudio(&tData);
	return 1;
}

void shutdownSoundEffectHandler() {
	return;
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
	return -1;
	Buffer b = fileToBuffer(tPath);
	return addBufferToSoundEffectHandler(b);
}

static int gDummy;

int loadSoundEffectFromBuffer(const Buffer& tBuffer) {
	return -1;
	Buffer ownedBuffer = copyBuffer(tBuffer);
	return addBufferToSoundEffectHandler(ownedBuffer);
}

void unloadSoundEffect(int tID) {
	return;
	SoundEffectEntry* e = &gSoundEffectData.mAllocatedChunks[tID];
	unloadSoundEffectEntry(e);
	gSoundEffectData.mAllocatedChunks.erase(tID);
}

static void tryEraseChannelChunk(int tChannel) {
	setProfilingSectionMarkerCurrentFunction();
	if (stl_map_contains(gSoundEffectData.mChunks, tChannel)) {
		DrakonTerminateAudio(&gSoundEffectData.mChunks[tChannel]);
		gSoundEffectData.mChunks.erase(tChannel);
	}
}

int playSoundEffect(int tID) {
	return -1;
	setProfilingSectionMarkerCurrentFunction();
	return playSoundEffectChannel(tID, -1, getSoundEffectVolume());
}

static int findEmptyChannel()
{
	return 1;
}

int playSoundEffectChannel(int tID, int tChannel, double tVolume, double /*tFreqMul*/, int /*tIsLooping*/)
{
	return -1;
	setProfilingSectionMarkerCurrentFunction();
	if (tChannel == -1)
	{
		tChannel = findEmptyChannel();
	}

	tryEraseChannelChunk(tChannel);

	DrakonAudioHandler& audioHandler = gSoundEffectData.mChunks[tChannel];
	SoundEffectEntry* e = &gSoundEffectData.mAllocatedChunks[tID];
	DrakonInitializeAudio(&audioHandler);
	DrakonLoadWavFromMemory(&audioHandler, e->mBuffer.mData, e->mBuffer.mLength, AUDIO_OUT_MAIN);
	DrakonPlayAudio(&audioHandler);
	logErrorFormat("Playing sound effect on channel %d with size %d", tChannel, e->mBuffer.mLength);

	return tChannel;
}

void stopSoundEffect(int tChannel) {
	return;
	setProfilingSectionMarkerCurrentFunction();
	if (gSoundEffectData.mChunks.find(tChannel) != gSoundEffectData.mChunks.end())
	{
		DrakonStopAudio(&gSoundEffectData.mChunks[tChannel]);
	}
	tryEraseChannelChunk(tChannel);
}

static void stopSingleSoundEffectCB(int tChannel, DrakonAudioHandler& tAudioHandler) {
	setProfilingSectionMarkerCurrentFunction();
	DrakonStopAudio(&tAudioHandler);
	tryEraseChannelChunk(tChannel);
}

void stopAllSoundEffects() {
	return;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gSoundEffectData.mChunks, stopSingleSoundEffectCB);
}

void panSoundEffect(int tChannel, double tPanning)
{
	return;
	setProfilingSectionMarkerCurrentFunction();
	const uint8_t right = uint8_t(std::min(std::max(tPanning, 0.0), 1.0) * 255);
}

int isSoundEffectPlayingOnChannel(int tChannel) {
	return 0;
	setProfilingSectionMarkerCurrentFunction();
	auto it = gSoundEffectData.mChunks.find(tChannel);
	if (it == gSoundEffectData.mChunks.end()) return 0;
	auto& audioHandler = it->second;
	return !DrakonGetAudioStatus(&audioHandler);
}

double getSoundEffectVolume() {
	return gSoundEffectData.mVolume;
}

void setSoundEffectVolume(double tVolume) {
	setProfilingSectionMarkerCurrentFunction();
	gSoundEffectData.mVolume = tVolume;
}
