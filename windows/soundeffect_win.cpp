#include "prism/soundeffect.h"

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <fmod/api/core/inc/fmod.hpp>
#include <fmod/api/core/inc/fmod_errors.h>
#elif defined _WIN32
#include <FMOD Studio API Windows/api/core/inc/fmod.hpp>
#include <FMOD Studio API Windows/api/core/inc/fmod_errors.h>
#endif

#include <algorithm>

#include "prism/file.h"
#include "prism/sound.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/stlutil.h"
#include "prism/log.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;
namespace prism {

	typedef struct {
		Buffer mBuffer;
		FMOD::Sound* mLoadedSound;
	} SoundEffectLoaded;

	static struct {
		double mVolume;
		map<int, SoundEffectLoaded> mLoaded;
		std::vector<FMOD::Channel*> mSfxChannels;
	} gSoundEffectData;

	extern FMOD::System* getPrismFmodSystem();
	extern int getPrismSoundChannelCount();

#ifdef _WIN32
	static void imguiSoundEffectsPlaying()
	{
		if (ImGui::TreeNode("Playing SoundEffects"))
		{
			ImGui::Columns(7, "SfxColumns");
			ImGui::Text("Slot"); ImGui::NextColumn();
			ImGui::Text("Ptr"); ImGui::NextColumn();
			ImGui::Text("Playing"); ImGui::NextColumn();
			ImGui::Text("Paused"); ImGui::NextColumn();
			ImGui::Text("Volume"); ImGui::NextColumn();
			ImGui::Text("Freq"); ImGui::NextColumn();
			ImGui::Text("Pos (ms)"); ImGui::NextColumn();
			ImGui::Separator();

			for (size_t i = 0; i < gSoundEffectData.mSfxChannels.size(); ++i)
			{
				FMOD::Channel* ch = gSoundEffectData.mSfxChannels[i];

				ImGui::Text("%zu", i);
				ImGui::NextColumn();

				if (!ch)
				{
					ImGui::Text("nullptr");
					ImGui::NextColumn();
					ImGui::Text("-"); ImGui::NextColumn();
					ImGui::Text("-"); ImGui::NextColumn();
					ImGui::Text("-"); ImGui::NextColumn();
					ImGui::Text("-"); ImGui::NextColumn();
					ImGui::Text("-"); ImGui::NextColumn();
					continue;
				}

				ImGui::Text("%p", (void*)ch);
				ImGui::NextColumn();

				bool playing = false;
				ch->isPlaying(&playing);

				if (playing)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
				else
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

				ImGui::Text("%s", playing ? "Yes" : "No");
				ImGui::NextColumn();

				ImGui::PopStyleColor();

				bool paused = false;
				ch->getPaused(&paused);
				ImGui::Text("%s", paused ? "Yes" : "No");
				ImGui::NextColumn();

				float volume = 0.0f;
				ch->getVolume(&volume);
				ImGui::Text("%.2f", volume);
				ImGui::NextColumn();

				float freq = 0.0f;
				ch->getFrequency(&freq);
				ImGui::Text("%.1f", freq);
				ImGui::NextColumn();

				unsigned int pos = 0;
				ch->getPosition(&pos, FMOD_TIMEUNIT_MS);
				ImGui::Text("%u", pos);
				ImGui::NextColumn();
			}

			ImGui::Columns(1);
			ImGui::TreePop();
		}
	}

	static void imguiSoundEffectsLoaded()
	{
		if (ImGui::TreeNode("Loaded SoundEffects"))
		{
			for (auto& e : gSoundEffectData.mLoaded)
			{
				ImGui::Text("ID: %d", e.first); ImGui::SameLine();
				ImGui::Text("Size: %d", e.second.mBuffer.mLength);
			}
			ImGui::TreePop();
		}

	}

	static void imguiSoundEffectData() {
		ImGui::Text("Volume: %f", gSoundEffectData.mVolume);
		imguiSoundEffectsLoaded();
		imguiSoundEffectsPlaying();
	}

	void imguiSoundEffectsHardware() {
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "SoundEffects HW", &isWindowShown);
		if (isWindowShown)
		{
			ImGui::Begin("SoundEffects HW", &isWindowShown);
			imguiSoundEffectData();
			ImGui::End();
		}
	}
#endif

	void initSoundEffects() {
		gSoundEffectData.mVolume = 1.0;
		gSoundEffectData.mSfxChannels.resize(getPrismSoundChannelCount() - 1, nullptr);
	}

	void setupSoundEffectHandler() {
		gSoundEffectData.mLoaded.clear();
	}

	static void unloadLoadedSoundEffect(SoundEffectLoaded* e) {
		FMOD_RESULT result = e->mLoadedSound->release();
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to unload sound effect: %s", FMOD_ErrorString(result));
		}
		freeBuffer(e->mBuffer);
	}

	static int unloadSingleSoundEffect(void* tCaller, SoundEffectLoaded& tData) {
		(void)tCaller;
		SoundEffectLoaded* e = &tData;
		unloadLoadedSoundEffect(e);
		return 1;
	}

	void shutdownSoundEffectHandler() {
		stopAllSoundEffects();

		stl_int_map_remove_predicate(gSoundEffectData.mLoaded, unloadSingleSoundEffect);
		gSoundEffectData.mLoaded.clear();
	}

	void setSoundEffectCompression(int /*tIsEnabled*/) {} // no need for compression in web/windows

	static int addBufferToSoundEffectHandler(Buffer tBuffer) {
		SoundEffectLoaded e;
		e.mBuffer = tBuffer;
		FMOD_CREATESOUNDEXINFO createSoundInfo{ sizeof(FMOD_CREATESOUNDEXINFO) , e.mBuffer.mLength };
		FMOD_RESULT result = getPrismFmodSystem()->createSound((char*)tBuffer.mData, FMOD_CREATESAMPLE | FMOD_OPENMEMORY | FMOD_LOOP_NORMAL, &createSoundInfo, &e.mLoadedSound);
		if (result != FMOD_OK)
		{
			logErrorFormat("Unable to load sound effect: %s", FMOD_ErrorString(result));
		}
		return stl_int_map_push_back(gSoundEffectData.mLoaded, e);
	}

	int loadSoundEffect(const char* tPath) {
		Buffer b = fileToBuffer(tPath);
		return addBufferToSoundEffectHandler(b);
	}

	static int gDummy;

	int loadSoundEffectFromBuffer(const Buffer& tBuffer) {

		Buffer ownedBuffer = copyBuffer(tBuffer);
		return addBufferToSoundEffectHandler(ownedBuffer);
	}

	void unloadSoundEffect(int tID) {
		auto e = &gSoundEffectData.mLoaded[tID];
		unloadLoadedSoundEffect(e);
		gSoundEffectData.mLoaded.erase(tID);
	}

	int playSoundEffect(int tID) {
		setProfilingSectionMarkerCurrentFunction();
		return playSoundEffectChannel(tID, -1, getSoundEffectVolume());
	}

	static bool isSlotFree(int i) {
		if (!gSoundEffectData.mSfxChannels[i]) return true;
		bool playing = false;
		if (gSoundEffectData.mSfxChannels[i]->isPlaying(&playing) != FMOD_OK) return true;
		return !playing;
	}

	static int allocSlot(int requested) {
		if (requested >= 0)
		{
			return requested % (int)gSoundEffectData.mSfxChannels.size();
		}

		for (int i = 0; i < (int)gSoundEffectData.mSfxChannels.size(); ++i)
		{
			if (isSlotFree(i)) return i;
		}

		logWarning("Unable to find free sound effect slot, defauting to 0");
		return 0;
	}

	int playSoundEffectChannel(int tID, int tChannel, double tVolume, double tFreqMul, int tIsLooping)
	{
		setProfilingSectionMarkerCurrentFunction();
		auto e = &gSoundEffectData.mLoaded[tID];
		
		const int slot = allocSlot(tChannel);
		if (!isSlotFree(slot) && gSoundEffectData.mSfxChannels[slot])
		{
			gSoundEffectData.mSfxChannels[slot]->stop();
			gSoundEffectData.mSfxChannels[slot] = nullptr;
		}

		FMOD_RESULT r = getPrismFmodSystem()->playSound(e->mLoadedSound, nullptr, true, &gSoundEffectData.mSfxChannels[slot]);
		if (r != FMOD_OK || !gSoundEffectData.mSfxChannels[slot]) return -1;

		gSoundEffectData.mSfxChannels[slot]->setVolume((float)tVolume);

		float base = 0.0f;
		gSoundEffectData.mSfxChannels[slot]->getFrequency(&base);
		gSoundEffectData.mSfxChannels[slot]->setFrequency(base * (float)tFreqMul);
		gSoundEffectData.mSfxChannels[slot]->setLoopCount(tIsLooping ? -1 : 0);
		gSoundEffectData.mSfxChannels[slot]->setPaused(false);

		return slot;
	}

	bool isSlotValidAndActive(int tChannel)
	{
		return tChannel < gSoundEffectData.mSfxChannels.size() && !isSlotFree(tChannel);
	}

	void stopSoundEffect(int tChannel) {
		setProfilingSectionMarkerCurrentFunction();
		if (!isSlotValidAndActive(tChannel)) return;
		gSoundEffectData.mSfxChannels[tChannel]->stop();
	}

	void stopAllSoundEffects() {
		setProfilingSectionMarkerCurrentFunction();
		for (int channel = 0; channel < int(gSoundEffectData.mSfxChannels.size()); channel++)
		{
			stopSoundEffect(channel);
		}
	}

	void panSoundEffect(int tChannel, double tPanning)
	{
		setProfilingSectionMarkerCurrentFunction();
		if (!isSlotValidAndActive(tChannel)) return;
		gSoundEffectData.mSfxChannels[tChannel]->setPan((float)tPanning);
	}

	int isSoundEffectPlayingOnChannel(int tChannel) {
		setProfilingSectionMarkerCurrentFunction();
		return isSlotValidAndActive(tChannel);
	}

	double getSoundEffectVolume() {
		return gSoundEffectData.mVolume;
	}

	void setSoundEffectVolume(double tVolume) {
		setProfilingSectionMarkerCurrentFunction();
		gSoundEffectData.mVolume = tVolume;
		for (int channel = 0; channel < int(gSoundEffectData.mSfxChannels.size()); channel++)
		{
			if (isSlotValidAndActive(channel))
			{
				gSoundEffectData.mSfxChannels[channel]->setVolume((float)tVolume);
			}
		}
	}

}