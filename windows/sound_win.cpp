#include "prism/sound.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL/SDL_mixer.h>
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
	Mix_Chunk* mTrackChunk;
	int mTrackChannel;	
	uint64_t mTimeWhenMusicPlaybackStarted;

	Microphone mMicrophone;
} gData;

void initSound() {
	gData.mPanning = 128;
	Mix_Init(MIX_INIT_OGG);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_AllocateChannels(1024);

	gData.mTrackChannel = -1;
	gData.mHasLoadedTrack = 0;
	gData.mIsPlayingTrack = 0;

	setVolume(0.1);
	gData.mMicrophone.mIsMicrophoneActive = 0;
}

void shutdownSound() {
	Mix_CloseAudio();
}

double getVolume() {
	return gData.mVolume / 128.0;
}

void setVolume(double tVolume) {
	gData.mVolume = (int)(tVolume * 128);
	Mix_Volume(-1, gData.mVolume);
}

double getPanningValue() {
	return (gData.mPanning / 128.0) - 1.0;
}

static void playMusicPath(char* tPath) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);

	Buffer b = fileToBuffer(fullPath);
	SDL_RWops* rwOps = SDL_RWFromConstMem(b.mData, b.mLength);
	gData.mTrackChunk = Mix_LoadWAV_RW(rwOps, 0);
	freeBuffer(b);
	gData.mHasLoadedTrack = 1;
}

static void loadTrack(int tTrack) {
	assert(!gData.mHasLoadedTrack);

	char path[1024];
	sprintf(path, "tracks/%d.wav", tTrack);
	playMusicPath(path);
}

static void unloadTrack() {
	assert(gData.mHasLoadedTrack);

	Mix_FreeChunk(gData.mTrackChunk);
	gData.mHasLoadedTrack = 0;
}

static void streamMusicFileGeneral(char* tPath, int tLoopAmount);

static void playTrackGeneral(int tTrack, int tLoopAmount) {
#ifdef __EMSCRIPTEN__
	//return;
#endif

	if (gData.mIsPlayingTrack) stopTrack();
	if (gData.mHasLoadedTrack) unloadTrack();

	char path[1024];
	sprintf(path, "tracks/%d.wav", tTrack);
	streamMusicFileGeneral(path, tLoopAmount);
}

void playTrack(int tTrack) {
	playTrackGeneral(tTrack, -1);
}

void stopTrack()
{
	if (!gData.mIsPlayingTrack) return;

	Mix_HaltChannel(gData.mTrackChannel);
}

void pauseTrack()
{
	if (!gData.mIsPlayingTrack || gData.mIsPaused) return;
	Mix_Pause(gData.mTrackChannel);
	gData.mIsPaused = 1;
}

void resumeTrack()
{
	if (!gData.mIsPlayingTrack || !gData.mIsPaused) return;

	Mix_Resume(gData.mTrackChannel);
	gData.mIsPaused = 0;
}

void playTrackOnce(int tTrack)
{
	playTrackGeneral(tTrack, 0);
}

static void musicFinishedCB(int tChannel) {
	if (tChannel != gData.mTrackChannel) return;
	gData.mTrackChannel = -1;
	gData.mIsPlayingTrack = 0;

	Mix_FreeChunk(gData.mTrackChunk);
	gData.mHasLoadedTrack = 0;
}

static void streamMusicFileGeneral(char* tPath, int tLoopAmount) {
	playMusicPath(tPath);
	Mix_ChannelFinished(musicFinishedCB);

	gData.mTrackChannel = Mix_PlayChannel(-1, gData.mTrackChunk, tLoopAmount);
	gData.mTimeWhenMusicPlaybackStarted = SDL_GetTicks();

	gData.mIsPaused = 0;
	gData.mIsPlayingTrack = 1;
}

void streamMusicFile(char * tPath)
{
	streamMusicFileGeneral(tPath, -1);
}

void streamMusicFileOnce(char * tPath)
{
	streamMusicFileGeneral(tPath, 0);
}

void stopStreamingMusicFile()
{
	stopTrack();
}

uint64_t getStreamingSoundTimeElapsedInMilliseconds()
{
	if (!gData.mIsPlayingTrack) return 0;
	if (!gData.mTimeWhenMusicPlaybackStarted) return 0;
	
	uint64_t now = SDL_GetTicks();
	return (uint64_t)(now - gData.mTimeWhenMusicPlaybackStarted);
}

int isPlayingStreamingMusic()
{
	return gData.mIsPlayingTrack;
}

void stopMusic()
{
	stopTrack();
}


static void microphoneCB(void *userdata, Uint8 *stream, int len) {
	(void)userdata;

	int i;
	for (i = 0; i < len; i++) {
		gData.mMicrophone.mSamplePointer = (gData.mMicrophone.mSamplePointer + 1) % MICROPHONE_SAMPLE_AMOUNT;

		if (gData.mMicrophone.mSampleAmount > MICROPHONE_SAMPLE_AMOUNT) {
			gData.mMicrophone.mSampleSum -= gData.mMicrophone.mSamples[gData.mMicrophone.mSamplePointer];
			gData.mMicrophone.mSampleAmount--;
		}

		gData.mMicrophone.mSamples[gData.mMicrophone.mSamplePointer] = stream[i];
		gData.mMicrophone.mSampleSum += stream[i];
		gData.mMicrophone.mSampleAmount++;
	}

	gData.mMicrophone.mMasterPeakVolume = 0;
	for (i = 0; i < MICROPHONE_SAMPLE_AMOUNT; i++) {
		gData.mMicrophone.mMasterPeakVolume = max(gData.mMicrophone.mMasterPeakVolume, (int)gData.mMicrophone.mSamples[i]);
	}
}


static void startMicrophone(void* tData)
{
	(void)tData;
	SDL_AudioSpec want, have;

	SDL_zero(want);
	want.freq = 44100;
	want.format = AUDIO_U8;
	want.channels = 1;
	want.samples = 256;
	want.callback = microphoneCB;
	gData.mMicrophone.mMicrophone = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), 1, &want, &have, 0);
	logg("Opening audio device");
	logString(SDL_GetAudioDeviceName(0, 1));
	if (have.format != want.format) {
		logError("We didn't get the wanted format.");
		recoverFromError();
		return;
	}

	SDL_PauseAudioDevice(gData.mMicrophone.mMicrophone, 0);

	gData.mMicrophone.mSampleSum = 0;
	gData.mMicrophone.mSamplePointer = 0;
	gData.mMicrophone.mSampleAmount = 0;
	gData.mMicrophone.mIsMicrophoneActive = 1;
	gData.mMicrophone.mMasterPeakVolume = 0;
}

static void stopMicrophone(void* tData)
{
	(void)tData;
	SDL_CloseAudioDevice(gData.mMicrophone.mMicrophone);

	gData.mMicrophone.mIsMicrophoneActive = 0;
}

ActorBlueprint getMicrophoneHandlerActorBlueprint()
{
	return makeActorBlueprint(startMicrophone, stopMicrophone);
}

double getMicrophoneVolume()
{
	return gData.mMicrophone.mMasterPeakVolume / 255.0;


}