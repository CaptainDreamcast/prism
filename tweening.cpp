#include "prism/tweening.h"

#include <stdio.h>

#include <prism/datastructures.h>
#include <prism/memoryhandler.h>
#include <prism/math.h>
#include <prism/stlutil.h>

#ifdef _WIN32
#include <vector>
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;

namespace prism {

	typedef struct {
		float* mDst;
		TweeningFunction mFunc;

		float mStart;
		float mEnd;

		Duration mNow;
		Duration mDuration;

		TweeningCBFunction mCB;
		void* mCaller;
	} Tween;

	static struct {
		map<int, Tween> mTweens;
		int mIsActive;
	} gTweening;

#ifdef _WIN32
	static std::vector<int> gImguiTweensToRemove;

	void imguiTweeningHandler()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Tweening", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Tweening", &isWindowShown);
		ImGui::Text("IsActive = %d", gTweening.mIsActive);
		ImGui::Text("Active Tweens = %d", (int)gTweening.mTweens.size());
		ImGui::Separator();

		gImguiTweensToRemove.clear();
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
		if (ImGui::BeginTable("Tweens", 6, flags, ImVec2(0, 260)))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Value");
			ImGui::TableSetupColumn("Start");
			ImGui::TableSetupColumn("End");
			ImGui::TableSetupColumn("Progress");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			for (auto& entryPair : gTweening.mTweens)
			{
				const int id = entryPair.first;
				Tween& e = entryPair.second;
				ImGui::PushID(id);
				ImGui::TableNextRow(); ImGui::TableNextColumn();
				ImGui::Text("%d", id); ImGui::TableNextColumn();
				ImGui::Text("%.3f", e.mDst ? *e.mDst : 0.0); ImGui::TableNextColumn();
				ImGui::Text("%.3f", e.mStart); ImGui::TableNextColumn();
				ImGui::Text("%.3f", e.mEnd); ImGui::TableNextColumn();
				const float fraction = e.mDuration > 0 ? (float)(e.mNow / e.mDuration) : 0.0f;
				ImGui::ProgressBar(fraction); ImGui::TableNextColumn();
				if (ImGui::SmallButton("Remove")) gImguiTweensToRemove.push_back(id);
				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		for (int id : gImguiTweensToRemove) removeTween(id);
		gImguiTweensToRemove.clear();
		ImGui::End();
	}
#endif

	static void unloadTweening(void*);
	static void loadTweening(void*)
	{
		setProfilingSectionMarkerCurrentFunction();

		if (gTweening.mIsActive) {
			unloadTweening(NULL);
		}
		gTweening.mTweens.clear();
		gTweening.mIsActive = 1;
	}

	static void unloadTweening(void*)
	{
		setProfilingSectionMarkerCurrentFunction();
		gTweening.mTweens.clear();
		gTweening.mIsActive = 0;
	}

	static int updateTween(void* tCaller, Tween& tData) {
		(void)tCaller;
		Tween* e = &tData;

		int isOver = handleDurationAndCheckIfOver(&e->mNow, e->mDuration);

		float baseT = getDurationPercentage(e->mNow, e->mDuration);
		float t = e->mFunc(baseT);

		*e->mDst = e->mStart + t * (e->mEnd - e->mStart);

		if (isOver && e->mCB) {
			e->mCB(e->mCaller);
		}

		return isOver;
	}

	static void updateTweening(void*)
	{
		setProfilingSectionMarkerCurrentFunction();
		stl_int_map_remove_predicate(gTweening.mTweens, updateTween);
	}

	ActorBlueprint getTweeningHandler() {
		return makeActorBlueprint(loadTweening, unloadTweening, updateTweening);
	}

	int tweenDouble(float* tDst, float tStart, float tEnd, TweeningFunction tFunc, Duration tDuration, TweeningCBFunction tCB, void* tCaller)
	{
		Tween e;
		e.mDst = tDst;
		e.mStart = tStart;
		e.mEnd = tEnd;
		e.mFunc = tFunc;
		e.mNow = 0;
		e.mDuration = tDuration;
		e.mCB = tCB;
		e.mCaller = tCaller;

		*tDst = tStart;

		return stl_int_map_push_back(gTweening.mTweens, e);
	}

	void removeTween(int tID)
	{
		gTweening.mTweens.erase(tID);
	}

	float linearTweeningFunction(float t) {
		return t;
	}

	float quadraticTweeningFunction(float t) {
		return t * t;
	}

	float inverseQuadraticTweeningFunction(float t) {
		return 1 - quadraticTweeningFunction(1 - t);
	}

	float squareRootTweeningFunction(float t) {
		return std::sqrt(t);
	}

	float overshootTweeningFunction(float t) {
		float overshoot = 1.5;
		if (t < 0.8f) return linearTweeningFunction((t / 0.8f) * overshoot);
		else return linearTweeningFunction(overshoot + ((t - 0.8f) / 0.2f) * (1.0f - overshoot));
	}

	float transformAtEndTweeningFunction(float t)
	{
		if (t >= 1) return 1;
		else return 0;
	}

}