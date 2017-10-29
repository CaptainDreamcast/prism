#include "tari/sound.h"

#include <assert.h>

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_mixer.h>
#elif defined _WIN32
#include <SDL_mixer.h>
#endif

#include "tari/log.h"
#include "tari/file.h"
#include "tari/datastructures.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"


#define MICROPHONE_SAMPLE_AMOUNT  50000


typedef struct {
	int mIsMicrophoneActive;
	SDL_AudioDeviceID mMicrophone;

	int mSampleAmount;
	uint8_t mSamples[MICROPHONE_SAMPLE_AMOUNT];
	int mSampleSum;
	int mSamplePointer;
} Microphone;

static struct {

	int mVolume;
	int mPanning;

	int mHasLoadedTrack;
	int mIsPlayingTrack;
	int mIsPaused;
	Mix_Chunk* mTrackChunk;
	int mTrackChannel;
	
	Microphone mMicrophone;
} gData;

void initSound() {
	gData.mVolume = 255;
	gData.mPanning = 128;
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_AllocateChannels(1024);

	gData.mHasLoadedTrack = 0;
	gData.mIsPlayingTrack = 0;

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

static void loadTrack(int tTrack) {
	assert(!gData.mHasLoadedTrack);

	char path[1024];
	char fullPath[1024];
	sprintf(path, "tracks/%d.wav", tTrack);
	getFullPath(fullPath, path);
	gData.mTrackChunk = Mix_LoadWAV(fullPath);
	gData.mHasLoadedTrack = 1;
}

static void unloadTrack() {
	assert(gData.mHasLoadedTrack);

	Mix_FreeChunk(gData.mTrackChunk);
	gData.mHasLoadedTrack = 0;
}

void playTrack(int tTrack) {
	if (gData.mIsPlayingTrack) stopTrack();
	if (gData.mHasLoadedTrack) unloadTrack();
	
	loadTrack(tTrack);
	gData.mTrackChannel = Mix_PlayChannel(-1, gData.mTrackChunk, -1);
	gData.mIsPaused = 0;
	gData.mIsPlayingTrack = 1;
}

void stopTrack()
{
	if (!gData.mIsPlayingTrack) return;

	Mix_HaltChannel(gData.mTrackChannel);
	gData.mIsPlayingTrack = 0;
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
}


static void startMicrophone(void* tData)
{

	SDL_AudioSpec want, have;

	SDL_zero(want);
	want.freq = 44100;
	want.format = AUDIO_S16SYS;
	want.channels = 1;
	want.samples = 1024;
	want.callback = microphoneCB;
	gData.mMicrophone.mMicrophone = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), 1, &want, &have, 0);
	logg("Opening audio device");
	logString(SDL_GetAudioDeviceName(0, 1));
	if (have.format != want.format) {
		logError("We didn't get the wanted format.");
		abortSystem();
		return;
	}

	SDL_PauseAudioDevice(gData.mMicrophone.mMicrophone, 0);

	gData.mMicrophone.mSampleSum = 0;
	gData.mMicrophone.mSamplePointer = 0;
	gData.mMicrophone.mSampleAmount = 0;
	gData.mMicrophone.mIsMicrophoneActive = 1;
}

static void stopMicrophone(void* tData)
{
	SDL_CloseAudioDevice(gData.mMicrophone.mMicrophone);

	gData.mMicrophone.mIsMicrophoneActive = 0;
}

static ActorBlueprint MicrophoneHandler = {
	.mLoad = startMicrophone,
	.mUnload = stopMicrophone,
};

ActorBlueprint getMicrophoneHandlerActorBlueprint()
{
	return MicrophoneHandler;
}

double getMicrophoneVolume()
{
	double avg = gData.mMicrophone.mSampleSum / (double)gData.mMicrophone.mSampleAmount;
	return avg / 255.0;
}