#include "prism/soundeffect.h"

#include <kos.h>
#include <dc/sound/sound.h>

#include "prism/file.h"
#include "prism/sound.h"
#include "prism/memoryhandler.h"

static struct {
	int mIsCompressing;
} gSoundEffectDreamcastData;

void initSoundEffects() {

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

static Buffer downsampleBufferIfNecessary(Buffer tBuffer) {

	auto header = (WaveHeader*)tBuffer.mData;
	if (header->mBitSize != 8) return tBuffer;

	uint32_t totalLength = sizeof(WaveHeader) + header->mLen / 2;
	void* fullData = allocMemory(totalLength);
	auto newHeader = (WaveHeader*)fullData;
	*newHeader = *header;
	newHeader->mFormat = 20;
	newHeader->mBitSize = 4;
	newHeader->mLen /= 2;

	uint8_t* src = ((uint8_t*)tBuffer.mData) + sizeof(WaveHeader);
	uint8_t* dst = ((uint8_t*)fullData) + sizeof(WaveHeader);

	int dstPos = 0;
	for (size_t srcPos = 0; srcPos < header->mLen; srcPos +=2) {
		uint8_t val1 = src[srcPos + 0] >> 4;
		uint8_t val2 = src[srcPos + 1] >> 4;

		dst[dstPos] = val1 | (val2 << 4);
		dstPos += 1;
	}

	return makeBufferOwned(fullData, totalLength);
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
	return snd_sfx_play(tID, 254, 128); // using 255 for volume results in sound effects glitching out
}

void stopSoundEffect(int tSFX) {
	if (tSFX == -1) return;
	snd_sfx_stop(tSFX);
}

void setSoundEffectVolume(double tVolume) {
	(void) tVolume;
}
