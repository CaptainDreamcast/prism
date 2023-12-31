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
#include "prism/memoryhandler.h"

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

typedef struct {
	uint8_t mBuffer1[8]; // 8
	uint32_t mMagic; // 12
	uint8_t mBuffer2[8]; // 20
	uint16_t mFormat; // 22
	uint16_t mStereo; // 24
	uint32_t mHertz; // 28
	uint8_t mBuffer3[6]; //34
	uint16_t mBitSize; //36
	uint8_t mBuffer4[4]; //40
	uint32_t mLen; // 44
} WaveHeader;

static Buffer upsampleBuffer8Bit(const Buffer& tBuffer) {
	auto header = (WaveHeader*)tBuffer.mData;
	uint32_t totalLength = sizeof(WaveHeader) + header->mLen * 2;

	void* fullData = allocMemory(totalLength);
	auto newHeader = (WaveHeader*)fullData;
	*newHeader = *header;
	newHeader->mBitSize = 16;
	newHeader->mLen *= 2;

	uint8_t* src = ((uint8_t*)tBuffer.mData) + sizeof(WaveHeader);
	int16_t* dst = (int16_t*)(((uint8_t*)fullData) + sizeof(WaveHeader));

	int dstPos = 0;
	size_t copyLength = header->mLen - (header->mLen % 2);
	for (size_t srcPos = 0; srcPos < copyLength; srcPos++) {
		dst[dstPos] = (int16_t(src[srcPos]) - 128) << 8;
		dstPos += 1;
	}

	return makeBufferOwned(fullData, totalLength);
}

static Buffer downsampleBuffer32Bit(const Buffer& tBuffer) {
	auto header = (WaveHeader*)tBuffer.mData;
	uint32_t totalLength = sizeof(WaveHeader) + header->mLen / 2;

	void* fullData = allocMemory(totalLength);
	auto newHeader = (WaveHeader*)fullData;
	*newHeader = *header;
	newHeader->mFormat = 0x01;
	newHeader->mBitSize = 16;
	newHeader->mLen /= 2;

	float* src = (float*)(((uint8_t*)tBuffer.mData) + sizeof(WaveHeader));
	int16_t* dst = (int16_t*)(((uint8_t*)fullData) + sizeof(WaveHeader));

	int dstPos = 0;
	size_t baseLength = header->mLen / 2;
	size_t copyLength = baseLength - (baseLength % 2);
	for (size_t srcPos = 0; srcPos < copyLength; srcPos++) {
		float fval = src[srcPos] * 2 - 1;
		
		dst[dstPos] = int16_t(INT16_MAX * fval);
		dstPos += 1;
	}

	return makeBufferOwned(fullData, totalLength);
}

static Buffer resampleBufferIfNecessary(const Buffer& tBuffer) {

	auto header = (WaveHeader*)tBuffer.mData;
	switch (header->mBitSize) {
	case 8:
		return upsampleBuffer8Bit(tBuffer);
	case 32:
		return downsampleBuffer32Bit(tBuffer);
	default:
		return copyBuffer(tBuffer);
	}
}

int loadSoundEffectFromBuffer(const Buffer& tBuffer) {
	Buffer ownedBuffer = resampleBufferIfNecessary(tBuffer);
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
		DrakonTerminateAudio(&gSoundEffectData.mChunks[tChannel]);
		gSoundEffectData.mChunks.erase(tChannel);
	}
}

int playSoundEffect(int tID) {
	setProfilingSectionMarkerCurrentFunction();
	return playSoundEffectChannel(tID, -1, getSoundEffectVolume());
}

static int findEmptyChannel()
{
	for (int i = 1; i < 9; i++)
	{
		if (!isSoundEffectPlayingOnChannel(i)) return i;
	}
	return -1;
}

int playSoundEffectChannel(int tID, int tChannel, double tVolume, double /*tFreqMul*/, int /*tIsLooping*/)
{
	setProfilingSectionMarkerCurrentFunction();
	if (tChannel == -1)
	{
		tChannel = findEmptyChannel();
		if (tChannel == -1) return -1;
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
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gSoundEffectData.mChunks, stopSingleSoundEffectCB);
}

void panSoundEffect(int tChannel, double tPanning)
{
	setProfilingSectionMarkerCurrentFunction();
	const uint8_t right = uint8_t(std::min(std::max(tPanning, 0.0), 1.0) * 255);
}

int isSoundEffectPlayingOnChannel(int tChannel) {
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
