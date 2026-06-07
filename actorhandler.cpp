#include "prism/actorhandler.h"

#include "prism/datastructures.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/wrapper.h"

#ifdef _WIN32
#include <vector>
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

namespace prism {

	typedef struct {
		ActorBlueprint mBP;
		ListIterator mListEntry;

		int mID;
		void* mData;
		int mIsOwned;
		int mIsPausable;
	} Actor;

	static struct {
		IntMap mActors;
		List mSequentialActorList;
		int mIsInitialized;
	} gPrismActorHandlerData;

#ifdef _WIN32
	static std::vector<int> gImguiActorsToRemove;

	static void imguiActorEntry(void* tCaller, void* tData)
	{
		(void)tCaller;
		Actor* e = (Actor*)tData;
		ImGui::PushID(e->mID);
		ImGui::TableNextRow(); ImGui::TableNextColumn();
		ImGui::Text("%d", e->mID); ImGui::TableNextColumn();
		ImGui::Text("%d", e->mIsOwned); ImGui::TableNextColumn();
		bool pausable = e->mIsPausable != 0;
		if (ImGui::Checkbox("##pausable", &pausable)) e->mIsPausable = pausable ? 1 : 0;
		ImGui::TableNextColumn();
		ImGui::Text("%s%s%s",
			e->mBP.mUpdate ? "U" : "-",
			e->mBP.mDraw ? "D" : "-",
			e->mBP.mIsActive ? "A" : "-");
		ImGui::TableNextColumn();
		if (ImGui::SmallButton("Remove")) gImguiActorsToRemove.push_back(e->mID);
		ImGui::PopID();
	}

	void imguiActorHandler()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Actor Handler", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Actor Handler", &isWindowShown);
		ImGui::Text("IsInitialized = %d", gPrismActorHandlerData.mIsInitialized);
		ImGui::Text("Actors = %d", int_map_size(&gPrismActorHandlerData.mActors));
		ImGui::Text("Sequential List Size = %d", list_size(&gPrismActorHandlerData.mSequentialActorList));
		ImGui::TextDisabled("U=Update D=Draw A=IsActive callback present");
		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Actors", 5, flags))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Owned");
			ImGui::TableSetupColumn("Pausable");
			ImGui::TableSetupColumn("U/D/A");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			gImguiActorsToRemove.clear();
			int_map_map(&gPrismActorHandlerData.mActors, imguiActorEntry, NULL);
			ImGui::EndTable();

			for (int id : gImguiActorsToRemove) removeActor(id);
			gImguiActorsToRemove.clear();
		}
		ImGui::End();
	}
#endif

	ActorBlueprint makeActorBlueprint(LoadActorFunction tLoad, UnloadActorFunction tUnload, UpdateActorFunction tUpdate, DrawActorFunction tDraw, IsActorActiveFunction tIsActive) {
		ActorBlueprint ret;
		ret.mLoad = tLoad;
		ret.mUnload = tUnload;
		ret.mUpdate = tUpdate;
		ret.mDraw = tDraw;
		ret.mIsActive = tIsActive;
		return ret;
	}

	void setupActorHandler()
	{
		setProfilingSectionMarkerCurrentFunction();
		if (gPrismActorHandlerData.mIsInitialized) {
			logWarning("Actor handling already initialized.");
			shutdownActorHandler();
		}

		gPrismActorHandlerData.mActors = new_int_map();
		gPrismActorHandlerData.mSequentialActorList = new_list();
		gPrismActorHandlerData.mIsInitialized = 1;
	}

	static void unloadActor(Actor* e) {
		if (e->mBP.mUnload) e->mBP.mUnload(e->mData);

		if (e->mIsOwned) {
			freeMemory(e->mData);
		}

		int_map_remove(&gPrismActorHandlerData.mActors, e->mID);
	}

	static int removeActorCB(void* tCaller, void* tData) {
		(void)tCaller;
		Actor* e = (Actor*)tData;
		unloadActor(e);
		return 1;
	}

	void shutdownActorHandler()
	{
		setProfilingSectionMarkerCurrentFunction();
		list_remove_predicate_inverted(&gPrismActorHandlerData.mSequentialActorList, removeActorCB, NULL);
		delete_int_map(&gPrismActorHandlerData.mActors);
		delete_list(&gPrismActorHandlerData.mSequentialActorList);
		gPrismActorHandlerData.mIsInitialized = 0;
	}

	static int updateSingleActor(void* tCaller, void* tData) {
		(void)tCaller;
		Actor* e = (Actor*)tData;
		if (isWrapperPaused() && e->mIsPausable) return 0;

		int isActive = e->mBP.mIsActive == NULL || e->mBP.mIsActive(e->mData);
		if (!isActive) {
			unloadActor(e);
			return 1;
		}
		if (e->mBP.mUpdate) e->mBP.mUpdate(e->mData);
		return 0;
	}

	void updateActorHandler()
	{
		setProfilingSectionMarkerCurrentFunction();
		list_remove_predicate(&gPrismActorHandlerData.mSequentialActorList, updateSingleActor, NULL);
	}

	static void drawSingleActor(void* tCaller, void* tData) {
		(void)tCaller;
		Actor* e = (Actor*)tData;

		if (e->mBP.mDraw) e->mBP.mDraw(e->mData);
	}

	void drawActorHandler()
	{
		setProfilingSectionMarkerCurrentFunction();
		list_map(&gPrismActorHandlerData.mSequentialActorList, drawSingleActor, NULL);
	}

	int instantiateActor(const ActorBlueprint& tBP)
	{
		return instantiateActorWithData(tBP, NULL, 0);
	}

	int instantiateActorWithData(const ActorBlueprint& tBP, void* tData, int tIsOwned)
	{
		Actor* e = (Actor*)allocMemory(sizeof(Actor));
		e->mBP = tBP;
		e->mData = tData;
		e->mIsOwned = tIsOwned;
		e->mIsPausable = 1;

		if (e->mBP.mLoad) e->mBP.mLoad(e->mData);

		list_push_back_owned(&gPrismActorHandlerData.mSequentialActorList, e);
		e->mListEntry = list_iterator_end(&gPrismActorHandlerData.mSequentialActorList);

		e->mID = int_map_push_back(&gPrismActorHandlerData.mActors, e);
		return e->mID;
	}

	void performOnActor(int tID, ActorInteractionFunction tFunc, void* tCaller)
	{
		Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
		tFunc(tCaller, e->mData);
	}

	void setActorUnpausable(int tID)
	{
		Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
		e->mIsPausable = 0;
	}

	void removeActor(int tID)
	{
		Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
		unloadActor(e);
		list_iterator_remove(&gPrismActorHandlerData.mSequentialActorList, e->mListEntry);
	}

	void* getActorData(int tID)
	{
		Actor* e = (Actor*)int_map_get(&gPrismActorHandlerData.mActors, tID);
		return e->mData;
	}

}