#include "prism/sound.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>

extern "C"
{
#include <DrakonSound/DrakonSound.h>
}

#include "prism/log.h"
#include "prism/file.h"
#include "prism/datastructures.h"
#include "prism/system.h"
#include "prism/math.h"

#include "prism/soundeffect.h"

using namespace std;

static struct {

	double mVolume;
	int mPanning;

	int mHasLoadedTrack;
	int mIsPlayingTrack;
	int mIsPaused;

	DrakonAudioHandler mAudio;

	uint64_t mTimeWhenMusicPlaybackStarted;
} gPrismWindowsSoundData;

void initSound() {
	gPrismWindowsSoundData.mPanning = 128;

	gPrismWindowsSoundData.mHasLoadedTrack = 0;
	gPrismWindowsSoundData.mIsPlayingTrack = 0;
	
	setVolume(0.2);
}

void shutdownSound() {

}

double getVolume() {
	return gPrismWindowsSoundData.mVolume;
}

void setVolume(double tVolume) {
	gPrismWindowsSoundData.mVolume = tVolume;
	DrakonSetVolume(gPrismWindowsSoundData.mVolume);
}

double getPanningValue() {
	return (gPrismWindowsSoundData.mPanning / 128.0) - 1.0;
}

void setPanningValue(int tChannel, double tPanning)
{
	//Mix_SetPanning(tChannel, tPanning);
}

static void playMusicPath(const char* tPath, int tIsLooping) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);
	std::string appPath = std::string("app0:/") + fullPath;
	DrakonInitializeAudio(&gPrismWindowsSoundData.mAudio);
	DrakonLoadOgg(&gPrismWindowsSoundData.mAudio, appPath.c_str(), AUDIO_OUT_BGM, tIsLooping);
	gPrismWindowsSoundData.mHasLoadedTrack = 1;
}

static void unloadTrack() {
	assert(gPrismWindowsSoundData.mHasLoadedTrack);

	DrakonTerminateAudio(&gPrismWindowsSoundData.mAudio);
	gPrismWindowsSoundData.mHasLoadedTrack = 0;
}

static void streamMusicFileGeneral(const char* tPath, int tIsLooping);

static void playTrackGeneral(int tTrack, int tIsLooping) {
	if (gPrismWindowsSoundData.mIsPlayingTrack) stopTrack();
	if (gPrismWindowsSoundData.mHasLoadedTrack) unloadTrack();

	char path[1024];
	sprintf(path, "tracks/%d.wav", tTrack);
	streamMusicFileGeneral(path, tIsLooping);
}

void playTrack(int tTrack) {
	playTrackGeneral(tTrack, 1);
}

void stopTrack()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack) return;
	DrakonStopAudio(&gPrismWindowsSoundData.mAudio);
}

void pauseTrack()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack || gPrismWindowsSoundData.mIsPaused) return;
	DrakonStopAudio(&gPrismWindowsSoundData.mAudio);
	gPrismWindowsSoundData.mIsPaused = 1;
}

void resumeTrack()
{
	if (!gPrismWindowsSoundData.mIsPlayingTrack || !gPrismWindowsSoundData.mIsPaused) return;

	DrakonPlayAudio(&gPrismWindowsSoundData.mAudio);
	gPrismWindowsSoundData.mIsPaused = 0;
}

void playTrackOnce(int tTrack)
{
	playTrackGeneral(tTrack, 0);
}

static void streamMusicFileGeneral(const char* tPath, int tIsLooping) {
	if (gPrismWindowsSoundData.mIsPlayingTrack)
	{
		unloadTrack();
	}

	playMusicPath(tPath, tIsLooping);

	DrakonPlayAudio(&gPrismWindowsSoundData.mAudio);
	gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted = getSystemTicks();

	gPrismWindowsSoundData.mIsPaused = 0;
	gPrismWindowsSoundData.mIsPlayingTrack = 1;
}

void streamMusicFile(const char * tPath)
{
	streamMusicFileGeneral(tPath, 1);
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
	
	uint64_t now = getSystemTicks();
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


static void startMicrophone(void* tData)
{

}

static void stopMicrophone(void* tData)
{

}

ActorBlueprint getMicrophoneHandlerActorBlueprint()
{
	return makeActorBlueprint(startMicrophone, stopMicrophone);
}

double getMicrophoneVolume()
{
	return 0.0;
}