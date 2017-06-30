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

static struct {

	int mVolume;
	int mPanning;

	int mHasLoadedTrack;
	int mIsPlayingTrack;
	int mIsPaused;
	Mix_Chunk* mTrackChunk;
	int mTrackChannel;
} gData;

void initSound() {
	gData.mVolume = 255;
	gData.mPanning = 128;
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_AllocateChannels(1024);

	gData.mHasLoadedTrack = 0;
	gData.mIsPlayingTrack = 0;
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
	gData.mTrackChannel = Mix_PlayChannel(-1, gData.mTrackChunk, 0);
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


