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

namespace prism {

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
		char mRiffIdentifier[4]; // 4
		uint32_t m_SizeMinus8; // 8
		char mMagic[4]; // 12
		char mFormatIdentifier[4]; // 16
		uint32_t mFormatSize; // 20
		uint16_t mFormat; // 22
		uint16_t mStereo; // 24
		uint32_t mHertz; // 28
		uint8_t mBuffer3[6]; //34
		uint16_t mBitSize; //36
	} WaveHeader;

	typedef struct
	{
		char mDataMagic[4]; //4
		uint32_t mLen; // 8
	} WaveDataHeader;

	static Buffer upsampleBuffer8Bit(const Buffer& tBuffer) {
		auto header = (WaveHeader*)tBuffer.mData;
		uint32_t dataHeaderOffset = header->mFormatSize + 20; // subchunk1 + subchunk1 header (8 bytes) + riff header (12 bytes)
		auto dataHeader = (WaveDataHeader*)((char*)tBuffer.mData + dataHeaderOffset); 
		uint32_t totalLength = dataHeaderOffset + sizeof(WaveDataHeader) + dataHeader->mLen * 2;

		void* fullData = allocMemory(totalLength);
		auto newHeader = (WaveHeader*)fullData;
		*newHeader = *header;
		newHeader->mBitSize = 16;
		auto newDataHeader = (WaveDataHeader*)((char*)fullData + dataHeaderOffset);
		newDataHeader->mLen = dataHeader->mLen * 2;

		uint8_t* src = ((uint8_t*)tBuffer.mData) + dataHeaderOffset + sizeof(WaveDataHeader);
		int16_t* dst = (int16_t*)((uint8_t*)fullData + dataHeaderOffset + sizeof(WaveDataHeader));

		int dstPos = 0;
		size_t copyLength = dataHeader->mLen - (dataHeader->mLen % 2);
		for (size_t srcPos = 0; srcPos < copyLength; srcPos++) {
			dst[dstPos] = (int16_t(src[srcPos]) - 128) << 8;
			dstPos += 1;
		}

		return makeBufferOwned(fullData, totalLength);
	}

	static Buffer downsampleBuffer32Bit(const Buffer& tBuffer) {
		auto header = (WaveHeader*)tBuffer.mData;
		uint32_t dataHeaderOffset = header->mFormatSize + 20; // subchunk1 + subchunk1 header (8 bytes) + riff header (12 bytes)
		auto dataHeader = (WaveDataHeader*)((char*)tBuffer.mData + dataHeaderOffset);
		uint32_t totalLength = dataHeaderOffset + sizeof(WaveDataHeader) + dataHeader->mLen / 2;

		void* fullData = allocMemory(totalLength);
		auto newHeader = (WaveHeader*)fullData;
		*newHeader = *header;
		newHeader->mFormat = 0x01;
		newHeader->mBitSize = 16;
		auto newDataHeader = (WaveDataHeader*)((char*)fullData + dataHeaderOffset);
		newDataHeader->mLen = dataHeader->mLen / 2;

		float* src = (float*)((uint8_t*)tBuffer.mData + dataHeaderOffset + sizeof(WaveDataHeader));
		int16_t* dst = (int16_t*)((uint8_t*)fullData + dataHeaderOffset + sizeof(WaveDataHeader));

		int dstPos = 0;
		size_t baseLength = dataHeader->mLen / 2;
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

	static bool checkSoundEffectBufferValidity(const Buffer& tBuffer)
	{
		const auto header = (WaveHeader*)tBuffer.mData;
		const auto magicString = (const char*)(&header->mMagic);
		return magicString[0] == 'W' && magicString[1] == 'A' && magicString[2] == 'V' && magicString[3] == 'E';
	}

	int loadSoundEffectFromBuffer(const Buffer& tBuffer) {
		if (!checkSoundEffectBufferValidity(tBuffer))
		{
			logWarning("[Soundeffect_Vita] Trying to load invalid sound effect buffer");
			return -1;
		}
		Buffer ownedBuffer = resampleBufferIfNecessary(tBuffer);
		return addBufferToSoundEffectHandler(ownedBuffer);
	}

	void unloadSoundEffect(int tID) {
		if (tID == -1) return;

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

}