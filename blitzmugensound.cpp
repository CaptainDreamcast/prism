#include "prism/blitzmugensound.h"

#include "prism/datastructures.h"
#include "prism/blitzentity.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

namespace prism {

	typedef struct {
		int mEntityID;
		MugenSounds* mSounds;
	} SoundEntry;

	static struct {
		IntMap mEntries;
	} gBlitzMugenSoundData;

#ifdef _WIN32
	static void imguiBlitzSoundEntry(void* tCaller, void* tData)
	{
		(void)tCaller;
		SoundEntry* e = (SoundEntry*)tData;
		ImGui::TableNextRow(); ImGui::TableNextColumn();
		ImGui::Text("%d", e->mEntityID); ImGui::TableNextColumn();
		ImGui::Text("%p", (void*)e->mSounds);
	}

	void imguiBlitzMugenSoundHandler()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Blitz", "Mugen Sound Handler", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Blitz Mugen Sound Handler", &isWindowShown);
		ImGui::Text("Entries = %d", int_map_size(&gBlitzMugenSoundData.mEntries));
		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("BlitzMugenSounds", 2, flags))
		{
			ImGui::TableSetupColumn("Entity ID");
			ImGui::TableSetupColumn("Sounds Ptr");
			ImGui::TableHeadersRow();
			int_map_map(&gBlitzMugenSoundData.mEntries, imguiBlitzSoundEntry, NULL);
			ImGui::EndTable();
		}
		ImGui::End();
	}
#endif

	static void loadBlitzMugenSoundHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		gBlitzMugenSoundData.mEntries = new_int_map();
	}

	static void unregisterEntity(int tEntityID);

	static BlitzComponent getBlitzMugenSoundComponent() {
		return makeBlitzComponent(unregisterEntity);
	}

	ActorBlueprint getBlitzMugenSoundHandler() {
		return makeActorBlueprint(loadBlitzMugenSoundHandler);
	}

	void addBlitzMugenSoundComponent(int tEntityID, MugenSounds* tSounds)
	{
		SoundEntry* e = (SoundEntry*)allocMemory(sizeof(SoundEntry));
		e->mEntityID = tEntityID;
		e->mSounds = tSounds;

		registerBlitzComponent(tEntityID, getBlitzMugenSoundComponent());
		int_map_push_owned(&gBlitzMugenSoundData.mEntries, tEntityID, e);
	}

	static SoundEntry* getSoundEntry(int tEntityID) {
		if (!int_map_contains(&gBlitzMugenSoundData.mEntries, tEntityID)) {
			logErrorFormat("Entity with ID %d does not have a mugen sound component.", tEntityID);
			recoverFromError();
		}

		return (SoundEntry*)int_map_get(&gBlitzMugenSoundData.mEntries, tEntityID);
	}

	void playEntityMugenSound(int tEntityID, int tGroup, int tSample)
	{
		SoundEntry* e = getSoundEntry(tEntityID);
		playMugenSound(e->mSounds, tGroup, tSample);
	}

	static void unregisterEntity(int tEntityID) {
		int_map_remove(&gBlitzMugenSoundData.mEntries, tEntityID);
	}

}