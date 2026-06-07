#include "prism/sound.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <fmod/api/core/inc/fmod.hpp>
#include <fmod/api/core/inc/fmod_errors.h>
#elif defined _WIN32
#include <FMOD Studio API Windows/api/core/inc/fmod.hpp>
#include <FMOD Studio API Windows/api/core/inc/fmod_errors.h>
#endif

#include "prism/log.h"
#include "prism/file.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"

#define MICROPHONE_SAMPLE_AMOUNT 128

#include "prism/soundeffect.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;
namespace prism {

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

		int mChannelCount;

		double mVolume;
		double mPanning;

		int mHasLoadedTrack;
		int mIsPlayingTrack;
		bool mShouldUnloadTrack;
		int mIsPaused;

		FMOD::System* mSystem;
		FMOD::Sound* mTrack;
		FMOD::Channel* mChannel;
		Buffer mTrackBuffer; // TODO: fix crash when this is unloaded between screens

		uint64_t mTimeWhenMusicPlaybackStarted;
		int mMusicChannel;

		Microphone mMicrophone;
	} gPrismWindowsSoundData;

	FMOD::System* getPrismFmodSystem() { return gPrismWindowsSoundData.mSystem; }
	int getPrismSoundChannelCount() { return gPrismWindowsSoundData.mChannelCount; }

#ifdef _WIN32
	static void imguiPrismWindowsSoundData() {
		if (ImGui::TreeNode("Sound Data"))
		{
			ImGui::Text("Volume: %f", gPrismWindowsSoundData.mVolume);
			ImGui::Text("Panning: %f", gPrismWindowsSoundData.mPanning);
			ImGui::Text("Has Loaded Track: %d", gPrismWindowsSoundData.mHasLoadedTrack);
			ImGui::Text("Is Playing Track: %d", gPrismWindowsSoundData.mIsPlayingTrack);
			ImGui::Text("Is Paused: %d", gPrismWindowsSoundData.mIsPaused);
			ImGui::TreePop();
		}
	}

	static void imguiPrismWindowsSoundMicrophone() {
		if (ImGui::TreeNode("Microphone"))
		{
			ImGui::Text("Is Microphone Active: %d", gPrismWindowsSoundData.mMicrophone.mIsMicrophoneActive);
			ImGui::Text("Sample Amount: %d", gPrismWindowsSoundData.mMicrophone.mSampleAmount);
			ImGui::Text("Sample Sum: %d", gPrismWindowsSoundData.mMicrophone.mSampleSum);
			ImGui::Text("Sample Pointer: %d", gPrismWindowsSoundData.mMicrophone.mSamplePointer);
			ImGui::Text("Master Peak Volume: %d", gPrismWindowsSoundData.mMicrophone.mMasterPeakVolume);
			ImGui::TreePop();
		}
	}

	void imguiSoundHardware() {
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Sound HW", &isWindowShown);
		if (isWindowShown)
		{
			ImGui::Begin("Drawing HW", &isWindowShown);
			ImGui::Text("Volume: %f", getVolume());
			ImGui::Text("Panning: %f", getPanningValue());
			ImGui::Text("Microphone Volume: %f", getMicrophoneVolume());
			imguiPrismWindowsSoundData();
			imguiPrismWindowsSoundMicrophone();
			ImGui::End();
		}
	}
#endif

	void initSound() {

		gPrismWindowsSoundData.mChannelCount = 64;

		gPrismWindowsSoundData.mPanning = 0;
		
		void* extraDriverData = nullptr;
		FMOD_RESULT result = FMOD::System_Create(&gPrismWindowsSoundData.mSystem);
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to create FMOD System: %s", FMOD_ErrorString(result));
		}

		result = gPrismWindowsSoundData.mSystem->init(gPrismWindowsSoundData.mChannelCount, FMOD_INIT_NORMAL, extraDriverData);
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to init FMOD System: %s", FMOD_ErrorString(result));
		}

		gPrismWindowsSoundData.mHasLoadedTrack = 0;
		gPrismWindowsSoundData.mIsPlayingTrack = 0;
		gPrismWindowsSoundData.mShouldUnloadTrack = false;
		gPrismWindowsSoundData.mTrack = nullptr;
		gPrismWindowsSoundData.mChannel = nullptr;

		setVolume(0.2);
		gPrismWindowsSoundData.mMicrophone.mIsMicrophoneActive = 0;
	}

	void shutdownSound() {
		FMOD_RESULT result = gPrismWindowsSoundData.mSystem->close();
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to close FMOD System: %s", FMOD_ErrorString(result));
		}

		result = gPrismWindowsSoundData.mSystem->release();
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to release FMOD System: %s", FMOD_ErrorString(result));
		}
	}

	static void unloadTrack() {
		assert(gPrismWindowsSoundData.mHasLoadedTrack);

		FMOD_RESULT result = gPrismWindowsSoundData.mTrack->release();
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to unload track: %s", FMOD_ErrorString(result));
		}

		freeBuffer(gPrismWindowsSoundData.mTrackBuffer);
		gPrismWindowsSoundData.mHasLoadedTrack = 0;
	}

	void updateSound() {
		gPrismWindowsSoundData.mSystem->update();
		if (gPrismWindowsSoundData.mShouldUnloadTrack)
		{
			gPrismWindowsSoundData.mIsPlayingTrack = 0;
			unloadTrack();
			gPrismWindowsSoundData.mShouldUnloadTrack = false;
		}
	}

	double getVolume() {
		return gPrismWindowsSoundData.mVolume;
	}

	void setVolume(double tVolume) {
		gPrismWindowsSoundData.mVolume = tVolume;
		if (gPrismWindowsSoundData.mIsPlayingTrack)
		{
			gPrismWindowsSoundData.mChannel->setVolume((float)gPrismWindowsSoundData.mVolume);
		}
	}

	double getPanningValue() {
		return gPrismWindowsSoundData.mPanning;
	}

	void setPanningValue(double tPanning)
	{
		gPrismWindowsSoundData.mPanning = tPanning;
		if (gPrismWindowsSoundData.mIsPlayingTrack)
		{
			gPrismWindowsSoundData.mChannel->setPan((float)gPrismWindowsSoundData.mPanning);
		}
	}

	static void playMusicPath(const char* tPath) {
		if (gPrismWindowsSoundData.mIsPlayingTrack) stopTrack();
		if (gPrismWindowsSoundData.mHasLoadedTrack) unloadTrack();
		gPrismWindowsSoundData.mShouldUnloadTrack = false;

		gPrismWindowsSoundData.mTrackBuffer = fileToBuffer(tPath);

		FMOD_CREATESOUNDEXINFO exInfo = {};
		exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exInfo.length = gPrismWindowsSoundData.mTrackBuffer.mLength;

		FMOD_RESULT result = gPrismWindowsSoundData.mSystem->createStream(
			(const char*)gPrismWindowsSoundData.mTrackBuffer.mData,
			FMOD_LOOP_NORMAL | FMOD_2D | FMOD_OPENMEMORY_POINT,
			&exInfo,
			&gPrismWindowsSoundData.mTrack);
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to create stream %s: %s", tPath, FMOD_ErrorString(result));
			freeBuffer(gPrismWindowsSoundData.mTrackBuffer);
			return;
		}

		gPrismWindowsSoundData.mHasLoadedTrack = 1;
	}

	static void streamMusicFileGeneral(const char* tPath, int tLoopAmount);

	static void playTrackGeneral(int tTrack, int tLoopAmount) {
#ifdef __EMSCRIPTEN__
		return;
#endif

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

		gPrismWindowsSoundData.mChannel->stop();
		gPrismWindowsSoundData.mIsPlayingTrack = 0;
	}

	void pauseTrack()
	{
		if (!gPrismWindowsSoundData.mIsPlayingTrack || gPrismWindowsSoundData.mIsPaused) return;
		gPrismWindowsSoundData.mChannel->setPaused(true);
		gPrismWindowsSoundData.mIsPaused = 1;
	}

	void resumeTrack()
	{
		if (!gPrismWindowsSoundData.mIsPlayingTrack || !gPrismWindowsSoundData.mIsPaused) return;

		gPrismWindowsSoundData.mChannel->setPaused(false);
		gPrismWindowsSoundData.mIsPaused = 0;
	}

	void playTrackOnce(int tTrack)
	{
		playTrackGeneral(tTrack, 0);
	}

	static FMOD_RESULT fmodCallback(FMOD_CHANNELCONTROL* /*channelcontrol*/, FMOD_CHANNELCONTROL_TYPE /*controltype*/, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype, void* /*commanddata1*/, void* /*commanddata2*/)
	{
		if (callbacktype == FMOD_CHANNELCONTROL_CALLBACK_END)
		{
			gPrismWindowsSoundData.mShouldUnloadTrack = true;
		}
		return FMOD_OK;
	}

	static void streamMusicFileGeneral(const char* tPath, int tLoopAmount) {
		playMusicPath(tPath);

		auto result = gPrismWindowsSoundData.mSystem->playSound(gPrismWindowsSoundData.mTrack, nullptr, false, &gPrismWindowsSoundData.mChannel);
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to play sound %s: %s", tPath, FMOD_ErrorString(result));
		}
		gPrismWindowsSoundData.mChannel->setCallback(fmodCallback);
		gPrismWindowsSoundData.mChannel->setVolume((float)gPrismWindowsSoundData.mVolume);
		gPrismWindowsSoundData.mChannel->setPan((float)gPrismWindowsSoundData.mPanning);
		gPrismWindowsSoundData.mChannel->setLoopCount(tLoopAmount);
		gPrismWindowsSoundData.mChannel->setPaused(false);

		gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted = SDL_GetTicks();

		gPrismWindowsSoundData.mIsPaused = 0;
		gPrismWindowsSoundData.mIsPlayingTrack = 1;
	}

	void streamMusicFile(const char* tPath)
	{
		streamMusicFileGeneral(tPath, -1);
	}

	void streamMusicFileOnce(const char* tPath)
	{
		streamMusicFileGeneral(tPath, 0);
	}

	void crossFadeMusicLayer(const char* tNewPath, bool tIsLooping)
	{
		if (!isPlayingStreamingMusic()) return;
		auto previousStartTime = gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted;
		auto timeMs = getStreamingSoundTimeElapsedInMilliseconds();
		stopStreamingMusicFile();
		streamMusicFileGeneral(tNewPath, tIsLooping ? -1 : 0);
		gPrismWindowsSoundData.mTimeWhenMusicPlaybackStarted = previousStartTime;
		gPrismWindowsSoundData.mChannel->setPosition((unsigned int)timeMs, FMOD_TIMEUNIT_MS);
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


	static void microphoneCB(void* userdata, Uint8* stream, int len) {
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
}