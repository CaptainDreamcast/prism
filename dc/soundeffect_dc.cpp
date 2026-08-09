#include "prism/soundeffect.h"

#include <cstring>
#include <set>

#include <kos.h>
#include <dc/sound/sound.h>
#include <dc/sound/aica_comm.h>

#include "prism/file.h"
#include "prism/sound.h"
#include "prism/memoryhandler.h"
#include "prism/math.h"

namespace prism {

	static struct {
		int mIsCompressing;

		float mVolume;

		std::set<int> mStereoEffectIds;
		bool mStereoChannel[64];
	} gSoundEffectDreamcastData;

	static bool isSoundFileStereo(const char* tPath) {
		uint8_t hdr[24];
		auto fh = fileOpen(tPath, O_RDONLY);
		if (fh == FILEHND_INVALID) return false;
		size_t read = fileRead(fh, hdr, sizeof(hdr));
		fileClose(fh);
		if (read < sizeof(hdr)) return false;
		uint16_t channels = (uint16_t)(hdr[22] | (hdr[23] << 8));
		return channels > 1;
	}

	static void trackSoundEffectChannel(int tID, int tChannel) {
		if (tChannel < 0 || tChannel >= 64) return;
		gSoundEffectDreamcastData.mStereoChannel[tChannel] = gSoundEffectDreamcastData.mStereoEffectIds.count(tID) > 0;
	}

	static void setChannelPan(int tChannel, float tPanning) {
		AICA_CMDSTR_CHANNEL(tmp, cmd, chan);
		cmd->cmd = AICA_CMD_CHAN;
		cmd->timestamp = 0;
		cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
		cmd->cmd_id = tChannel;
		chan->cmd = AICA_CH_CMD_UPDATE | AICA_CH_UPDATE_SET_PAN;
		chan->pan = (uint32)(tPanning * 127 + 128);
		snd_sh4_to_aica(tmp, cmd->size);
	}

	void initSoundEffects() {
		gSoundEffectDreamcastData.mVolume = 1.0;
	}

	void setupSoundEffectHandler() {

	}

	void shutdownSoundEffectHandler() {
		snd_sfx_unload_all();
		gSoundEffectDreamcastData.mStereoEffectIds.clear();
	}

	void setSoundEffectCompression(int tIsEnabled) {
		gSoundEffectDreamcastData.mIsCompressing = tIsEnabled;
	}

	int loadSoundEffect(const char* tPath) {
		char fullPath[1024];
		getFullPath(fullPath, tPath);
		if (snd_mem_available() >= getFileSize(tPath) * 2) {
			int id = snd_sfx_load(fullPath);
			if (id != SFXHND_INVALID && isSoundFileStereo(tPath)) {
				gSoundEffectDreamcastData.mStereoEffectIds.insert(id);
			}
			return id;
		}
		else {
			return -1;
		}
	}

	typedef struct {
		uint8_t mRiff[4]; // 4, "RIFF"
		uint32_t mTotalSize; // 8, file size - 8
		uint8_t mWaveFmt[8]; // 16, "WAVEfmt "
		uint32_t mFmtSize; // 20, 16 for the canonical headers this code handles
		uint16_t mFormat; // 22, 1 = PCM, 3 = float PCM, 0x14 = Yamaha/AICA ADPCM
		uint16_t mChannels; // 24
		uint32_t mHertz; // 28
		uint32_t mBytesPerSecond; // 32
		uint16_t mBlockAlign; // 34
		uint16_t mBitSize; // 36
		uint8_t mDataTag[4]; // 40, "data"
		uint32_t mLen; // 44, data chunk size in bytes
	} WaveHeader;

	// Yamaha/AICA 4-bit ADPCM. The AICA uses YMZ280B-style ADPCM with two quirks: nibbles are decoded low-first, and the per-step diff is clamped to 0..32767
	// KOS' wav2adpcm is the authority on how this needs to work
	static void stepYamahaAdpcm(uint8_t tNibble, int16_t* tHistory, int16_t* tStepSize) {
		static const int stepScaleTable[8] = { 230, 230, 230, 230, 307, 409, 512, 614 };
		const int delta = tNibble & 7;
		int diff = ((1 + (delta << 1)) * *tStepSize) >> 3;
		diff = clamp(diff, 0, 32767); // AICA-specific, not part of base YMZ280B ADPCM
		const int newValue = (tNibble & 8) ? (*tHistory - diff) : (*tHistory + diff);
		const int newStepSize = (stepScaleTable[delta] * *tStepSize) >> 8;
		*tHistory = (int16_t)clamp(newValue, -32768, 32767);
		*tStepSize = (int16_t)clamp(newStepSize, 127, 24576);
	}

	typedef int16_t(*SampleGetter)(const uint8_t* tData, size_t tIndex);

	static int16_t getSample8Bit(const uint8_t* tData, size_t tIndex) {
		return int16_t((int(tData[tIndex]) - 128) << 8); // 8-bit WAV is unsigned with a 128 bias
	}

	static int16_t getSample16Bit(const uint8_t* tData, size_t tIndex) {
		int16_t value; // 16-bit WAV is signed little-endian
		memcpy(&value, tData + tIndex * 2, 2);
		return value;
	}

	static int16_t getSample32BitInt(const uint8_t* tData, size_t tIndex) {
		int32_t value;
		memcpy(&value, tData + tIndex * 4, 4);
		return int16_t(value >> 16);
	}

	static int16_t getSample32BitFloat(const uint8_t* tData, size_t tIndex) {
		float value;
		memcpy(&value, tData + tIndex * 4, 4);
		return int16_t(fclamp(value, -1.0f, 1.0f) * 32767.0f);
	}

	static void encodeYamahaAdpcmChannel(uint8_t* tDst, const uint8_t* tSrc, size_t tSampleCount, size_t tChannels, size_t tChannelOffset, SampleGetter tGetSample) {
		int16_t history = 0;
		int16_t stepSize = 127;
		uint8_t lowNibble = 0;
		for (size_t i = 0; i < tSampleCount; i++) {
			const int sample = tGetSample(tSrc, i * tChannels + tChannelOffset) & ~7; // dropping the low bits reduces noise
			const int diff = sample - history;
			const int absDiff = (diff < 0) ? -diff : diff;
			uint8_t nibble = uint8_t(std::min((absDiff << 2) / stepSize, 7));
			if (diff < 0) nibble |= 8;
			// the AICA plays back the low nibble of each byte first
			if (!(i & 1)) lowNibble = nibble;
			else *tDst++ = lowNibble | (nibble << 4);
			stepYamahaAdpcm(nibble, &history, &stepSize); // track the decoder state for the next prediction
		}
	}

	static Buffer compressBufferToAdpcm(const Buffer& tBuffer, size_t tBytesPerSample, SampleGetter tGetSample) {
		const auto header = (WaveHeader*)tBuffer.mData;
		const size_t channels = header->mChannels;
		if (channels < 1 || channels > 2) return tBuffer; // KOS only supports mono/stereo

		size_t dataLength = header->mLen;
		if (dataLength > tBuffer.mLength - sizeof(WaveHeader)) dataLength = tBuffer.mLength - sizeof(WaveHeader); // guard against erroneous headers

		size_t samplesPerChannel = dataLength / tBytesPerSample / channels;
		samplesPerChannel -= samplesPerChannel % 2; // two samples per ADPCM byte
		if (!samplesPerChannel) return tBuffer;
		const size_t adpcmBytesPerChannel = samplesPerChannel / 2;
		const uint32_t totalLength = uint32_t(sizeof(WaveHeader) + adpcmBytesPerChannel * channels);
		if (snd_mem_available() < totalLength * 2) return tBuffer; // don't bother compressing if it doesn't fit

		void* fullData = allocMemory(totalLength);
		auto newHeader = (WaveHeader*)fullData;
		*newHeader = *header;
		newHeader->mFormat = 0x14; // Yamaha ADPCM, the variant KOS expects (stereo channels non-interleaved)
		newHeader->mBitSize = 4;
		newHeader->mLen = uint32_t(adpcmBytesPerChannel * channels);
		newHeader->mBlockAlign = uint16_t((channels * 4) / 8);
		newHeader->mBytesPerSecond = uint32_t((header->mHertz * channels * 4) / 8);
		newHeader->mTotalSize = totalLength - 8;

		const uint8_t* src = ((const uint8_t*)tBuffer.mData) + sizeof(WaveHeader);
		uint8_t* dst = ((uint8_t*)fullData) + sizeof(WaveHeader);
		for (size_t channel = 0; channel < channels; channel++) {
			// KOS stores stereo ADPCM planar: all left channel data, then all right channel data
			encodeYamahaAdpcmChannel(dst + channel * adpcmBytesPerChannel, src, samplesPerChannel, channels, channel, tGetSample);
		}

		return makeBufferOwned(fullData, totalLength);
	}

	static Buffer downsampleBufferIfNecessary(const Buffer& tBuffer) {
		if (tBuffer.mLength <= sizeof(WaveHeader)) return tBuffer;
		const auto header = (WaveHeader*)tBuffer.mData;
		if (memcmp(header->mRiff, "RIFF", 4) || memcmp(header->mDataTag, "data", 4) || header->mFmtSize != 16) return tBuffer; // only canonical headers supported
		if (header->mFormat != 1 && header->mFormat != 3) return tBuffer; // already ADPCM or unknown format

		switch (header->mBitSize) {
		case 8:
			return compressBufferToAdpcm(tBuffer, 1, getSample8Bit);
		case 16:
			return compressBufferToAdpcm(tBuffer, 2, getSample16Bit);
		case 32:
			return compressBufferToAdpcm(tBuffer, 4, (header->mFormat == 3) ? getSample32BitFloat : getSample32BitInt);
		default:
			return tBuffer;
		}
	}

	static int isCanonicalUncompressed8BitPcmBuffer(const Buffer& tBuffer) {
		if (tBuffer.mLength <= sizeof(WaveHeader)) return 0;
		const auto header = (WaveHeader*)tBuffer.mData;
		if (memcmp(header->mRiff, "RIFF", 4) || memcmp(header->mDataTag, "data", 4) || header->mFmtSize != 16) return 0;
		return header->mFormat == 1 && header->mBitSize == 8;
	}

	// Convert from WAV stored as 8-bit PCM unsigned (silence = 128) to KOS / AICA as signed 8-bit (silence = 0)
	static Buffer convert8BitBufferToSigned(const Buffer& tBuffer) {
		const auto header = (WaveHeader*)tBuffer.mData;
		size_t dataLength = header->mLen;
		if (dataLength > tBuffer.mLength - sizeof(WaveHeader)) dataLength = tBuffer.mLength - sizeof(WaveHeader); // guard against erroneous headers
		const uint32_t totalLength = uint32_t(sizeof(WaveHeader) + dataLength);
		void* fullData = allocMemory(totalLength);
		memcpy(fullData, tBuffer.mData, totalLength);
		auto newHeader = (WaveHeader*)fullData;
		newHeader->mLen = uint32_t(dataLength);
		uint8_t* samples = ((uint8_t*)fullData) + sizeof(WaveHeader);
		for (size_t i = 0; i < dataLength; i++) samples[i] ^= 0x80;
		return makeBufferOwned(fullData, totalLength);
	}

	int loadSoundEffectFromBuffer(const Buffer& tBuffer) {
		char tempPath[1024];
		strcpy(tempPath, "$/ram/tempsound.wav");

		Buffer b = makeBuffer(tBuffer.mData, tBuffer.mLength);
		if (gSoundEffectDreamcastData.mIsCompressing) {
			b = downsampleBufferIfNecessary(b);
		}
		if (isCanonicalUncompressed8BitPcmBuffer(b)) { // not converted if compression turned it into ADPCM
			Buffer converted = convert8BitBufferToSigned(b);
			freeBuffer(b);
			b = converted;
		}
		const auto header = (WaveHeader*)b.mData;
		if (snd_mem_available() < header->mLen * 2) { // final bailout check for non-compressed erroneous headers
			freeBuffer(b);
			return -1;
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
		gSoundEffectDreamcastData.mStereoEffectIds.erase(tID);
	}

	static int parseVolume(float tVolume) {
		return (int)(tVolume * 254);  // using 255 for volume results in sound effects glitching out);
	}

	int playSoundEffect(int tID) {
		if (tID == -1) return -1;
		auto chn = snd_sfx_play(tID, parseVolume(gSoundEffectDreamcastData.mVolume), 128, 1.0, 0);
		trackSoundEffectChannel(tID, chn);
		return chn;
	}

	int playSoundEffectChannel(int tID, int tChannel, float tVolume, float tFreqMul, int tIsLooping) {
		if (tID == -1) return -1;
		int chn;
		if (tChannel == -1)
		{
			chn = snd_sfx_play(tID, parseVolume(tVolume), 128, tFreqMul, tIsLooping);
		}
		else {
			chn = snd_sfx_play_chn(tChannel, tID, parseVolume(tVolume), 128, tFreqMul, tIsLooping);
		}
		trackSoundEffectChannel(tID, chn);
		return chn;
	}

	void stopSoundEffect(int tChannel) {
		if (tChannel == -1) return;
		snd_sfx_stop(tChannel);
	}

	void stopAllSoundEffects() {
		snd_sfx_stop_all();
	}

	void panSoundEffect(int tChannel, float tPanning) {
		if (tChannel == -1) return;
		setChannelPan(tChannel, tPanning);
		if (tChannel + 1 < 64 && gSoundEffectDreamcastData.mStereoChannel[tChannel]) {
			setChannelPan(tChannel + 1, tPanning);
		}
	}

	int isSoundEffectPlayingOnChannel(int tChannel) {
		return snd_sfx_chn_playing(tChannel);
	}

	void setSoundEffectVolume(float tVolume) {
		gSoundEffectDreamcastData.mVolume = tVolume;
	}

	float getSoundEffectVolume() {
		return gSoundEffectDreamcastData.mVolume;
	}
}