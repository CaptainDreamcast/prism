#include "prism/sound.h"

#include "prism/actorhandler.h"

namespace prism {

	static struct {
		float mVolume = 1.0;
		float mPanning = 0.0;
		int mIsPlayingStreaming = 0;
		uint64_t mStreamStartTime = 0;
	} gSoundLinuxData;

	void initSound() {}
	void shutdownSound() {}
	void updateSound() {}

	float getVolume() { return gSoundLinuxData.mVolume; }
	void setVolume(float tVolume) { gSoundLinuxData.mVolume = tVolume; }
	float getPanningValue() { return gSoundLinuxData.mPanning; }
	void setPanningValue(float tPanning) { gSoundLinuxData.mPanning = tPanning; }

	void playTrack(int /*tTrack*/) {}
	void stopTrack() {}
	void pauseTrack() {}
	void resumeTrack() {}
	void playTrackOnce(int /*tTrack*/) {}

	void streamMusicFile(const char* /*tPath*/) { gSoundLinuxData.mIsPlayingStreaming = 1; }
	void streamMusicFileOnce(const char* /*tPath*/) { gSoundLinuxData.mIsPlayingStreaming = 1; }
	void stopStreamingMusicFile() { gSoundLinuxData.mIsPlayingStreaming = 0; }
	uint64_t getStreamingSoundTimeElapsedInMilliseconds() { return 0; }
	int isPlayingStreamingMusic() { return gSoundLinuxData.mIsPlayingStreaming; }
	void stopMusic() { gSoundLinuxData.mIsPlayingStreaming = 0; }
	void pauseMusic() {}
	void resumeMusic() {}
	void crossFadeMusicLayer(const char* /*tNewPath*/, bool /*tIsLooping*/) {}

	static void loadMicrophoneHandler(void* /*tData*/) {}
	static void unloadMicrophoneHandler(void* /*tData*/) {}
	static void updateMicrophoneHandler(void* /*tData*/) {}

	ActorBlueprint getMicrophoneHandlerActorBlueprint() {
		return makeActorBlueprint(loadMicrophoneHandler, unloadMicrophoneHandler, updateMicrophoneHandler);
	}
	float getMicrophoneVolume() { return 0.0; }

}
