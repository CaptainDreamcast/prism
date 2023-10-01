#include "prism/sound.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL_mixer.h>
#elif defined _WIN32
#include <SDL_mixer.h>
#endif

#include "prism/log.h"
#include "prism/file.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"

#define MICROPHONE_SAMPLE_AMOUNT 128

#include "prism/soundeffect.h"

using namespace std;

typedef struct {
	int mIsMicrophoneActive;
	SDL_AudioDeviceID mMicrophone;

	int mSampleAmount;
	uint8_t mSamples[MICROPHONE_SAMPLE_AMOUNT];
	int mSampleSum;
	int mSamplePointer;

	int mMasterPeakVolume;
} Microphone;

static struct {

	int mVolume;
	int mPanning;

	int mHasLoadedTrack;
	int mIsPlayingTrack;
	int mIsPaused;
	Mix_Music* mTrackChunk;
	uint64_t mTimeWhenMusicPlaybackStarted;

	Microphone mMicrophone;
} gPrismWindowsSoundData;

void initSound() {
	gPrismWindowsSoundData.mPanning = 128;
	if (!Mix_Init(MIX_INIT_OGG))
	{
		logErrorFormat("Unable to init SDL Mixer: %s", SDL_GetError());
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		logErrorFormat("Unable to open audio: %s", SDL_GetError());
	}
	if (Mix_AllocateChannels(1024) != 1024)
	{
		logErrorFormat("Unable to allocate mixer channels: %s", SDL_GetError());
	}
	gPrismWindowsSoundData.mHasLoadedTrack = 0;
	gPrismWindowsSoundData.mIsPlayingTrack = 0;
	
	setVolume(0.2);
	gPrismWindowsSoundData.mMicrophone.mIsMicrophoneActive = 0;
}

void shutdownSound() {
	Mix_CloseAudio();
}

double getVolume() {
	return gPrismWindowsSoundData.mVolume / 128.0;
}

void setVolume(double tVolume) {
	gPrismWindowsSoundData.mVolume = (int)(tVolume * 128);
	Mix_VolumeMusic(gPrismWindowsSoundData.mVolume);
}

double getPanningValue() {
	return (gPrismWindowsSoundData.mPanning / 128.0) - 1.0;
}

void setPanningValue(int tChannel, double tPanning)
{
	const uint8_t right = uint8_t(std::min(std::max(tPanning, 0.0), 1.0) * 255);
	Mix_SetPanning(tChannel, 255 - right, right);
}

static void playMusicPath(const char* tPath) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);

	Buffer tBuffer = fileToBuffer(fullPath);
	SDL_RWops* rwOps = SDL_RWFromConstMem(tBuffer.mData, tBuffer.mLength);
	gPrismWindowsSoundData.mTrackChunk = Mix_LoadMUS_RW(rwOps, 1);
	if (!gPrismWindowsSoundData.mTrackChunk) {
		logErrorFormat("Unable to play sound %s: %s", tPath, SDL_GetError());
	}
	gPrismWindowsSoundData.mHasLoadedTrack = 1;
}

static void unloadTrack() {
	assert(gPrismWindowsSoundData.mHasLoadedTrack);

	Mix_FreeMusic(gPrismWindowsSoundData.mTrackChunk);
	gPrismWindowsSoundData.mHasLoadedTrack = 0;
}

static void streamMusicFileGeneral(const char* tPath, int tLoopAmount);

static void playTrackGeneral(int tTrack, int tLoopAmount) {
	return;

	if (gPrismWindowsSoundData.mIsPlayingTrack) stopTrack();
	if (gPrismWindowsSoundData.mHasLoadedTrack) unloadTrack();

	char path[1024];
	sprintf(path, "tracks/%d.wav", tTrack);
	streamMusicFileGeneral(path, tLoopAmount);
}

void playTrack(int tTrack) {
	playTrackGeneral(tTrack, -1);
}

void stopTrack()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack) return;

	Mix_HaltMusic();
}

void pauseTrack()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack || gPrismWindowsSoundData.mIsPaused) return;
	Mix_PauseMusic();
	gPrismWindowsSoundData.mIsPaused = 1;
}

void resumeTrack()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack || !gPrismWindowsSoundData.mIsPaused) return;

	Mix_ResumeMusic();
	gPrismWindowsSoundData.mIsPaused = 0;
}

void playTrackOnce(int tTrack)
{
	playTrackGeneral(tTrack, 0);
}

static void musicFinishedCB() {
	gPrismWindowsSoundData.mIsPlayingTrack = 0;

	Mix_FreeMusic(gPrismWindowsSoundData.mTrackChunk);
	gPrismWindowsSoundData.mHasLoadedTrack = 0;
}

static void streamMusicFileGeneral(const char* tPath, int tLoopAmount) {
	return;

	playMusicPath(tPath);
	Mix_HookMusicFinished(musicFinishedCB);

	Mix_PlayMusic(gPrismWindowsSoundData.mTrackChunk, tLoopAmount);
	gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted = SDL_GetTicks();

	gPrismWindowsSoundData.mIsPaused = 0;
	gPrismWindowsSoundData.mIsPlayingTrack = 1;
}

void streamMusicFile(const char * tPath)
{
	streamMusicFileGeneral(tPath, -1);
}

void streamMusicFileOnce(const char * tPath)
{
	streamMusicFileGeneral(tPath, 0);
}

void stopStreamingMusicFile()
{
	stopTrack();
}

uint64_t getStreamingSoundTimeElapsedInMilliseconds()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack) return 0;
	if (!gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted) return 0;
	
	uint64_t now = SDL_GetTicks();
	return (uint64_t)(now - gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted);
}

int isPlayingStreamingMusic()
{
	return gPrismWindowsSoundData.mIsPlayingTrack;
}

void stopMusic()
{
	stopTrack();
}

void pauseMusic()
{
	pauseTrack();
}

void resumeMusic()
{
	resumeTrack();
}


static void microphoneCB(void *userdata, Uint8 *stream, int len) {
	(void)userdata;

	int i;
	for (i = 0; i < len; i++) {
		gPrismWindowsSoundData.mMicrophone.mSamplePointer = (gPrismWindowsSoundData.mMicrophone.mSamplePointer + 1) % MICROPHONE_SAMPLE_AMOUNT;

		if (gPrismWindowsSoundData.mMicrophone.mSampleAmount > MICROPHONE_SAMPLE_AMOUNT) {
			gPrismWindowsSoundData.mMicrophone.mSampleSum -= gPrismWindowsSoundData.mMicrophone.mSamples[gPrismWindowsSoundData.mMicrophone.mSamplePointer];
			gPrismWindowsSoundData.mMicrophone.mSampleAmount--;
		}

		gPrismWindowsSoundData.mMicrophone.mSamples[gPrismWindowsSoundData.mMicrophone.mSamplePointer] = stream[i];
		gPrismWindowsSoundData.mMicrophone.mSampleSum += stream[i];
		gPrismWindowsSoundData.mMicrophone.mSampleAmount++;
	}

	gPrismWindowsSoundData.mMicrophone.mMasterPeakVolume = 0;
	for (i = 0; i < MICROPHONE_SAMPLE_AMOUNT; i++) {
		gPrismWindowsSoundData.mMicrophone.mMasterPeakVolume = max(gPrismWindowsSoundData.mMicrophone.mMasterPeakVolume, (int)gPrismWindowsSoundData.mMicrophone.mSamples[i]);
	}
}


static void startMicrophone(void* tData)
{
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	SDL_AudioSpec want, have;

	SDL_zero(want);
	want.freq = 44100;
	want.format = AUDIO_U8;
	want.channels = 1;
	want.samples = 256;
	want.callback = microphoneCB;
	gPrismWindowsSoundData.mMicrophone.mMicrophone = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), 1, &want, &have, 0);
	logg("Opening audio device");
	logString(SDL_GetAudioDeviceName(0, 1));
	if (have.format != want.format) {
		logError("We didn't get the wanted format.");
		recoverFromError();
		return;
	}

	SDL_PauseAudioDevice(gPrismWindowsSoundData.mMicrophone.mMicrophone, 0);

	gPrismWindowsSoundData.mMicrophone.mSampleSum = 0;
	gPrismWindowsSoundData.mMicrophone.mSamplePointer = 0;
	gPrismWindowsSoundData.mMicrophone.mSampleAmount = 0;
	gPrismWindowsSoundData.mMicrophone.mIsMicrophoneActive = 1;
	gPrismWindowsSoundData.mMicrophone.mMasterPeakVolume = 0;
}

static void stopMicrophone(void* tData)
{
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	SDL_CloseAudioDevice(gPrismWindowsSoundData.mMicrophone.mMicrophone);

	gPrismWindowsSoundData.mMicrophone.mIsMicrophoneActive = 0;
}

ActorBlueprint getMicrophoneHandlerActorBlueprint()
{
	return makeActorBlueprint(startMicrophone, stopMicrophone);
}

double getMicrophoneVolume()
{
	return gPrismWindowsSoundData.mMicrophone.mMasterPeakVolume / 255.0;
}