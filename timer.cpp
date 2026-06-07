#include "prism/timer.h"

#include "prism/memoryhandler.h"
#include "prism/datastructures.h"

#include "prism/stlutil.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;

namespace prism {

	typedef struct TimerElement_internal {
		Duration mNow;
		Duration mDuration;
		TimerCB mCB;
		void* mCaller;

	} TimerElement;

	static struct {
		map<int, TimerElement> mList;
	} gTimerData;

	void removeTimer(int tID);

#ifdef _WIN32
	void imguiTimer()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Timer", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Timer", &isWindowShown);
		ImGui::Text("Active Timers = %d", (int)gTimerData.mList.size());
		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Timers", 5, flags))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Now");
			ImGui::TableSetupColumn("Duration");
			ImGui::TableSetupColumn("Progress");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			int timerToRemove = -1;
			for (auto& entryPair : gTimerData.mList)
			{
				const int id = entryPair.first;
				TimerElement& e = entryPair.second;
				ImGui::PushID(id);
				ImGui::TableNextRow(); ImGui::TableNextColumn();
				ImGui::Text("%d", id); ImGui::TableNextColumn();
				ImGui::Text("%.1f", e.mNow); ImGui::TableNextColumn();
				ImGui::Text("%.1f", e.mDuration); ImGui::TableNextColumn();
				const float fraction = e.mDuration > 0 ? (float)(e.mNow / e.mDuration) : 0.0f;
				ImGui::ProgressBar(fraction); ImGui::TableNextColumn();
				if (ImGui::SmallButton("Remove")) timerToRemove = id;
				ImGui::PopID();
			}
			ImGui::EndTable();

			if (timerToRemove != -1) removeTimer(timerToRemove);
		}
		ImGui::End();
	}
#endif

	int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller) {
		TimerElement e;
		e.mNow = 0;
		e.mDuration = tDuration;
		e.mCB = tCB;
		e.mCaller = tCaller;

		return stl_int_map_push_back(gTimerData.mList, e);
	}

	void removeTimer(int tID)
	{
		gTimerData.mList.erase(tID);
	}

	void setupTimer() {
		gTimerData.mList.clear();
	}

	static int updateCB(void* tCaller, TimerElement& tData) {
		(void)tCaller;
		TimerElement* cur = &tData;
		int isOver = handleDurationAndCheckIfOver(&cur->mNow, cur->mDuration);

		if (isOver) {
			if (cur->mCB)
			{
				cur->mCB(cur->mCaller);
			}
			return 1;
		}

		return 0;
	}

	void updateTimer() {
		stl_int_map_remove_predicate(gTimerData.mList, updateCB);
	}

	void clearTimer() {
		gTimerData.mList.clear();
	}

	void shutdownTimer() {
		clearTimer();
	}

	int hasTimerFinished(int tID) {
		return gTimerData.mList.find(tID) == gTimerData.mList.end();
	}
}