#include "prism/physicshandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <map>

#include "prism/datastructures.h"
#include "prism/physics.h"
#include "prism/memoryhandler.h"
#include "prism/stlutil.h"

#ifdef _WIN32
#include <vector>
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;

namespace prism {

	static struct {
		int mIsActive;
		map<int, PhysicsHandlerElement> mList;
	} gPhysicsHandler;

#ifdef _WIN32
	static std::vector<PhysicsHandlerElement*> gImguiPhysicsToRemove;

	void imguiPhysicsHandler()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "Physics Handler", &isWindowShown);
		if (!isWindowShown) return;

		ImGui::Begin("Physics Handler", &isWindowShown);
		ImGui::Text("IsActive = %d", gPhysicsHandler.mIsActive);
		ImGui::Text("Elements = %d", (int)gPhysicsHandler.mList.size());
		ImGui::Separator();

		gImguiPhysicsToRemove.clear();
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
		if (ImGui::BeginTable("PhysicsElements", 6, flags, ImVec2(0, 260)))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Position");
			ImGui::TableSetupColumn("Velocity");
			ImGui::TableSetupColumn("Paused");
			ImGui::TableSetupColumn("Speed");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			for (auto& entryPair : gPhysicsHandler.mList)
			{
				PhysicsHandlerElement& e = entryPair.second;
				ImGui::PushID(e.mID);
				ImGui::TableNextRow(); ImGui::TableNextColumn();
				ImGui::Text("%d", e.mID); ImGui::TableNextColumn();
				ImGui::Text("%.1f %.1f %.1f", e.mObj.mPosition.x, e.mObj.mPosition.y, e.mObj.mPosition.z); ImGui::TableNextColumn();
				ImGui::Text("%.2f %.2f %.2f", e.mObj.mVelocity.x, e.mObj.mVelocity.y, e.mObj.mVelocity.z); ImGui::TableNextColumn();
				bool paused = e.mIsPaused != 0;
				if (ImGui::Checkbox("##paused", &paused)) e.mIsPaused = paused ? 1 : 0;
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(80);
				ImGui::DragScalar("##speed", ImGuiDataType_Double, &e.mTimeDilatation, 0.01f);
				ImGui::TableNextColumn();
				if (ImGui::SmallButton("Remove")) gImguiPhysicsToRemove.push_back(&e);
				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		for (PhysicsHandlerElement* e : gImguiPhysicsToRemove) removeFromPhysicsHandler(e);
		gImguiPhysicsToRemove.clear();
		ImGui::End();
	}
#endif

	void setupPhysicsHandler() {
		if (gPhysicsHandler.mIsActive) shutdownPhysicsHandler();
		gPhysicsHandler.mIsActive = 1;
		gPhysicsHandler.mList.clear();
	}

	void shutdownPhysicsHandler() {
		if (!gPhysicsHandler.mIsActive) return;
		stl_delete_map(gPhysicsHandler.mList);
		gPhysicsHandler.mIsActive = 0;

	}

	static void handleSinglePhysicsObjectInList(void* tCaller, PhysicsHandlerElement& tData) {
		(void)tCaller;
		PhysicsHandlerElement* data = &tData;
		if (data->mIsPaused) return;
		data->mTimeDilatationNow += data->mTimeDilatation;
		int updateAmount = (int)data->mTimeDilatationNow;
		data->mTimeDilatationNow -= updateAmount;
		while (updateAmount--) {
			Gravity prevGravity = getGravity();
			setGravity(data->mGravity);
			setMaxVelocityDouble(data->mMaxVelocity);
			setDragCoefficient(data->mDragCoefficient);
			handlePhysics(&data->mObj);
			resetMaxVelocity();
			resetDragCoefficient();
			setGravity(prevGravity);
		}
	}

	void updatePhysicsHandler() {
		stl_int_map_map(gPhysicsHandler.mList, handleSinglePhysicsObjectInList);
	}

	PhysicsHandlerElement* addToPhysicsHandler(const Position& tPosition) {
		PhysicsHandlerElement data;
		resetPhysicsObject(&data.mObj);
		data.mObj.mPosition = tPosition;
		data.mMaxVelocity = INFINITY;
		data.mDragCoefficient = Vector3D(0, 0, 0);
		data.mGravity = getGravity();
		data.mIsPaused = 0;
		data.mTimeDilatation = 1.0;
		data.mTimeDilatationNow = 0.0;

		const auto id = stl_int_map_push_back(gPhysicsHandler.mList, data);
		auto& element = gPhysicsHandler.mList[id];
		element.mID = id;
		return &element;
	}

	void removeFromPhysicsHandler(PhysicsHandlerElement* data) {
		gPhysicsHandler.mList.erase(data->mID);
	}

	PhysicsObject* getPhysicsFromHandler(PhysicsHandlerElement* data) {
		return &data->mObj;
	}

	Position getHandledPhysicsPosition(PhysicsHandlerElement* data)
	{
		return data->mObj.mPosition;
	}

	Position* getHandledPhysicsPositionReference(PhysicsHandlerElement* data) {
		return &data->mObj.mPosition;
	}

	Velocity* getHandledPhysicsVelocityReference(PhysicsHandlerElement* data)
	{
		return &data->mObj.mVelocity;
	}

	Acceleration* getHandledPhysicsAccelerationReference(PhysicsHandlerElement* data)
	{
		return &data->mObj.mAcceleration;
	}

	void addAccelerationToHandledPhysics(PhysicsHandlerElement* data, const Acceleration& tAccel) {
		if (data->mIsPaused) return;
		PhysicsObject* obj = &data->mObj;
		obj->mAcceleration = vecAdd(obj->mAcceleration, tAccel);
	}

	void stopHandledPhysics(PhysicsHandlerElement* data) {
		PhysicsObject* obj = &data->mObj;
		obj->mVelocity = Vector3D(0, 0, 0);
	}

	void pauseHandledPhysics(PhysicsHandlerElement* data)
	{
		data->mIsPaused = 1;
	}

	void resumeHandledPhysics(PhysicsHandlerElement* data)
	{
		data->mIsPaused = 0;
	}

	void setHandledPhysicsMaxVelocity(PhysicsHandlerElement* data, double tVelocity) {
		data->mMaxVelocity = tVelocity;
	}

	void setHandledPhysicsDragCoefficient(PhysicsHandlerElement* data, const Vector3D& tDragCoefficient) {
		data->mDragCoefficient = tDragCoefficient;
	}

	void setHandledPhysicsGravity(PhysicsHandlerElement* data, const Vector3D& tGravity) {
		data->mGravity = tGravity;
	}

	void setHandledPhysicsSpeed(PhysicsHandlerElement* data, double tSpeed) {
		data->mTimeDilatation = tSpeed;
	}

}