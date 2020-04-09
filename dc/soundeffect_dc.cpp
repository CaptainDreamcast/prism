#include "prism/soundeffect.h"

#include <kos.h>
#include <dc/sound/sound.h>

#include "prism/file.h"
#include "prism/sound.h"
#include "prism/memoryhandler.h"
#include "prism/math.h"

static struct {
	int mIsCompressing;

	int mVolume;
} gSoundEffectDreamcastData;

void initSoundEffects() {
	gSoundEffectDreamcastData.mVolume = 254; // using 255 for volume results in sound effects glitching out
}

void setupSoundEffectHandler() {
	
}

void shutdownSoundEffectHandler() {
	snd_sfx_unload_all();
}

void setSoundEffectCompression(int tIsEnabled) {
	gSoundEffectDreamcastData.mIsCompressing = tIsEnabled;
}

int loadSoundEffect(char* tPath) {
    char fullPath[1024];
	getFullPath(fullPath, tPath);
	if (snd_mem_available() >= getFileSize(tPath) * 2) {
		return snd_sfx_load(fullPath);
	}
	else {
		return -1;
	}
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

static Buffer downsampleBuffer8Bit(Buffer tBuffer) {
	auto header = (WaveHeader*)tBuffer.mData;
	uint32_t totalLength = sizeof(WaveHeader) + header->mLen / 2;
	if (snd_mem_available() < totalLength * 2) return tBuffer; // don't bother compressing if it doesn't fit

	void* fullData = allocMemory(totalLength);
	auto newHeader = (WaveHeader*)fullData;
	*newHeader = *header;
	newHeader->mFormat = 20;
	newHeader->mBitSize = 4;
	newHeader->mLen /= 2;

	uint8_t* src = ((uint8_t*)tBuffer.mData) + sizeof(WaveHeader);
	uint8_t* dst = ((uint8_t*)fullData) + sizeof(WaveHeader);

	int dstPos = 0;
	size_t copyLength = header->mLen - (header->mLen % 2);
	for (size_t srcPos = 0; srcPos < copyLength; srcPos += 2) {
		uint8_t val1 = src[srcPos + 0] >> 4;
		uint8_t val2 = src[srcPos + 1] >> 4;
		dst[dstPos] = val1 | (val2 << 4);
		dstPos += 1;
	}

	return makeBufferOwned(fullData, totalLength);
}

static Buffer downsampleBuffer16Bit(Buffer tBuffer) {
	auto header = (WaveHeader*)tBuffer.mData;
	uint32_t totalLength = sizeof(WaveHeader) + header->mLen / 4;
	if (snd_mem_available() < totalLength * 2) return tBuffer; // don't bother compressing if it doesn't fit

	void* fullData = allocMemory(totalLength);
	auto newHeader = (WaveHeader*)fullData;
	*newHeader = *header;
	newHeader->mFormat = 20;
	newHeader->mBitSize = 4;
	newHeader->mLen /= 4;

	uint16_t* src = (uint16_t*)(((uint8_t*)tBuffer.mData) + sizeof(WaveHeader));
	uint8_t* dst = ((uint8_t*)fullData) + sizeof(WaveHeader);

	int dstPos = 0;
	size_t baseLength = header->mLen / 2;
	size_t copyLength = baseLength - (baseLength % 2);
	for (size_t srcPos = 0; srcPos < copyLength; srcPos += 2) {
		uint16_t val1 = src[srcPos + 0] >> 12;
		uint16_t val2 = src[srcPos + 1] >> 12;

		dst[dstPos] = val1 | (val2 << 4);
		dstPos += 1;
	}

	return makeBufferOwned(fullData, totalLength);
}

static Buffer downsampleBuffer32Bit(Buffer tBuffer) {
	auto header = (WaveHeader*)tBuffer.mData;
	uint32_t totalLength = sizeof(WaveHeader) + header->mLen / 8;
	if (snd_mem_available() < totalLength * 2) return tBuffer; // don't bother compressing if it doesn't fit

	void* fullData = allocMemory(totalLength);
	auto newHeader = (WaveHeader*)fullData;
	*newHeader = *header;
	newHeader->mFormat = 20;
	newHeader->mBitSize = 4;
	newHeader->mLen /= 8;

	float* src = (float*)(((uint8_t*)tBuffer.mData) + sizeof(WaveHeader));
	uint8_t* dst = ((uint8_t*)fullData) + sizeof(WaveHeader);

	int dstPos = 0;
	size_t baseLength = header->mLen / 4;
	size_t copyLength = baseLength - (baseLength % 2);
	for (size_t srcPos = 0; srcPos < copyLength; srcPos += 2) {
		float fval1 = src[srcPos + 0];
		float fval2 = src[srcPos + 1];
		uint8_t val1 = clamp(int(15 * fval1), 0, 15);
		uint8_t val2 = clamp(int(15 * fval2), 0, 15);

		dst[dstPos] = val1 | (val2 << 4);
		dstPos += 1;
	}

	return makeBufferOwned(fullData, totalLength);
}

static Buffer downsampleBufferIfNecessary(Buffer tBuffer) {

	auto header = (WaveHeader*)tBuffer.mData;
	switch (header->mBitSize) {
	case 8:
		return downsampleBuffer8Bit(tBuffer);
	case 16:
		return downsampleBuffer16Bit(tBuffer);
	case 32:
		return downsampleBuffer32Bit(tBuffer);
	default:
		return tBuffer;
	}	
}

int loadSoundEffectFromBuffer(Buffer tBuffer) {
	char tempPath[1024];
	strcpy(tempPath, "$/ram/tempsound.wav");

	Buffer b = makeBuffer(tBuffer.mData, tBuffer.mLength);
	if (gSoundEffectDreamcastData.mIsCompressing) {
		b = downsampleBufferIfNecessary(b);
	}
	bufferToFile(tempPath, b);
	int ret = loadSoundEffect(tempPath);
	fileUnlink(tempPath);

	freeBuffer(b);
	return ret;
}

void unloadSoundEffect(int tID) {
	if (tID == -1) return;
	snd_sfx_unload(tID);
}

int playSoundEffect(int tID) {
	if (tID == -1) return -1;
	return snd_sfx_play(tID, gSoundEffectDreamcastData.mVolume, 128);
}

static int parseVolume(double tVolume) {
	return (int)(tVolume * 254);  // using 255 for volume results in sound effects glitching out);
}

int playSoundEffectChannel(int tID, int tChannel, double tVolume, double tFreqMul, int tIsLooping) {
	if (tID == -1) return -1;
	return snd_sfx_play_chn(tChannel, tID, parseVolume(tVolume), 128, tFreqMul, tIsLooping);
}

void stopSoundEffect(int tChannel) {
	if (tChannel == -1) return;
	snd_sfx_stop(tChannel);
}

void stopAllSoundEffects() {
	snd_sfx_stop_all();
}

void panSoundEffect(int tChannel, double tPanning) {
	if (tChannel == -1) return;
	snd_stream_pan(tChannel, int(tPanning * 127 + 128));
}

int isSoundEffectPlayingOnChannel(int tChannel) {
	return snd_sfx_chn_playing(tChannel);
}

void setSoundEffectVolume(double tVolume) {
	gSoundEffectDreamcastData.mVolume = parseVolume(tVolume);
}
