#include "prism/soundeffect.h"

#include <stdlib.h>

#include "prism/file.h"

namespace prism {

	static struct {
		int mNextID = 1;
		float mVolume = 1.0;
	} gSoundEffectLinuxData;

	void initSoundEffects() {}
	void setupSoundEffectHandler() {}
	void shutdownSoundEffectHandler() {}

	void setSoundEffectCompression(int /*tIsEnabled*/) {}

	int loadSoundEffect(const char* /*tPath*/) { return gSoundEffectLinuxData.mNextID++; }
	int loadSoundEffectFromBuffer(const Buffer& /*tBuffer*/) { return gSoundEffectLinuxData.mNextID++; }
	void unloadSoundEffect(int /*tID*/) {}

	int playSoundEffect(int /*tID*/) { return -1; }
	int playSoundEffectChannel(int /*tID*/, int tChannel, float /*tVolume*/, float /*tFreqMul*/, int /*tIsLooping*/) { return tChannel; }
	int playSoundEffectFile(const std::string& /*tPath*/) { return -1; }
	void stopSoundEffect(int /*tChannel*/) {}
	void stopAllSoundEffects() {}
	void panSoundEffect(int /*tChannel*/, float /*tPanning*/) {}
	int isSoundEffectPlayingOnChannel(int /*tChannel*/) { return 0; }

	float getSoundEffectVolume() { return gSoundEffectLinuxData.mVolume; }
	void setSoundEffectVolume(float tVolume) { gSoundEffectLinuxData.mVolume = tVolume; }

}
