#include "prism/blitzcamerahandler.h"

#include <cstring>

#include "prism/log.h"
#include "prism/math.h"
#include "prism/system.h"
#include "prism/geometry.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

namespace prism {

	static struct {
		int mIsActive;
		Position mBaseCameraPosition;
		Position mCameraPosition;
		Vector3D mScale;
		double mAngle;
		Position2D mEffectOffset;

		GeoRectangle2D mCameraRange;

		int mScreenShakeDurationLeft = 0;
		double mScreenShakeFrequency;
		int mScreenShakeAmplitude;
		double mScreenShakePhaseOffset;
		Vector2D mScreenShakeOffset;
	} gBlitzCameraHandlerData;

#ifdef _WIN32
	void imguiBlitzCameraHandler()
	{
		static bool isWindowShown = false;
		imguiPrismAddTab("Blitz", "Camera Handler", &isWindowShown);
		if (!isWindowShown) return;

		auto& d = gBlitzCameraHandlerData;
		ImGui::Begin("Blitz Camera Handler", &isWindowShown);
		ImGui::Text("IsActive = %d", d.mIsActive);
		Position basePos = d.mBaseCameraPosition;
		if (ImGui::DragScalarN("Base Position", ImGuiDataType_Double, &basePos.x, 2, 0.5f)) setBlitzCameraHandlerPosition(basePos);
		ImGui::Text("Camera Position = %.1f, %.1f", d.mCameraPosition.x, d.mCameraPosition.y);
		ImGui::DragScalarN("Scale", ImGuiDataType_Double, &d.mScale.x, 2, 0.01f);
		ImGui::DragScalar("Angle", ImGuiDataType_Double, &d.mAngle, 0.01f);
		ImGui::Text("Effect Offset = %.1f, %.1f", d.mEffectOffset.x, d.mEffectOffset.y);
		ImGui::Separator();
		ImGui::Text("Screen Shake Left = %d", d.mScreenShakeDurationLeft);
		ImGui::Text("Shake Offset = %.2f, %.2f", d.mScreenShakeOffset.x, d.mScreenShakeOffset.y);
		if (ImGui::SmallButton("Trigger Shake")) setBlitzCameraScreenShakeDefault();
		ImGui::End();
	}
#endif

	static void loadBlitzCameraHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();

		gBlitzCameraHandlerData.mBaseCameraPosition = Vector3D(0, 0, 0);
		gBlitzCameraHandlerData.mScreenShakeOffset = Vector2D(0, 0);
		gBlitzCameraHandlerData.mCameraPosition = Vector3D(0, 0, 0);
		gBlitzCameraHandlerData.mScale = Vector3D(1, 1, 1);
		gBlitzCameraHandlerData.mAngle = 0;
		ScreenSize sz = getScreenSize();
		gBlitzCameraHandlerData.mEffectOffset = Vector2D(sz.x / 2, sz.y / 2);
		gBlitzCameraHandlerData.mCameraRange = GeoRectangle2D(-INF / 2, -INF / 2, INF, INF);
		gBlitzCameraHandlerData.mIsActive = 1;
	}

	static double calculateShake(int t, double tPhaseOffset, double tFrequency, int tAmplitude) {
		return sin(tPhaseOffset + t * (tFrequency / 360.0) * 2.0 * M_PI) * tAmplitude;
	}

	static void calculateAndSetFinalCameraPosition() {
		gBlitzCameraHandlerData.mCameraPosition = gBlitzCameraHandlerData.mBaseCameraPosition + gBlitzCameraHandlerData.mScreenShakeOffset;
	}

	static void updateScreenShake() {
		if (!gBlitzCameraHandlerData.mScreenShakeDurationLeft) return;

		gBlitzCameraHandlerData.mScreenShakeDurationLeft--;
		if (!gBlitzCameraHandlerData.mScreenShakeDurationLeft) {
			gBlitzCameraHandlerData.mScreenShakeOffset = Vector2D(0, 0);
		}
		else {
			gBlitzCameraHandlerData.mScreenShakeOffset = Vector2D(0, calculateShake(gBlitzCameraHandlerData.mScreenShakeDurationLeft, gBlitzCameraHandlerData.mScreenShakePhaseOffset, gBlitzCameraHandlerData.mScreenShakeFrequency, gBlitzCameraHandlerData.mScreenShakeAmplitude));
		}
		calculateAndSetFinalCameraPosition();
	}

	static void updateBlitzCameraHandler(void*) {
		updateScreenShake();
	}

	ActorBlueprint getBlitzCameraHandler()
	{
		return makeActorBlueprint(loadBlitzCameraHandler, nullptr, updateBlitzCameraHandler);
	}

	int isBlitzCameraHandlerEnabled()
	{
		return gBlitzCameraHandlerData.mIsActive;
	}

	Position* getBlitzCameraHandlerPositionReference()
	{
		return &gBlitzCameraHandlerData.mCameraPosition;
	}

	Position getBlitzCameraHandlerPosition()
	{
		return gBlitzCameraHandlerData.mCameraPosition;
	}

	void setBlitzCameraHandlerPosition(const Position& tPos)
	{
		gBlitzCameraHandlerData.mBaseCameraPosition = Vector3D(tPos.x, tPos.y, 0);
		gBlitzCameraHandlerData.mBaseCameraPosition = clampPositionToGeoRectangle(gBlitzCameraHandlerData.mBaseCameraPosition, gBlitzCameraHandlerData.mCameraRange);
		calculateAndSetFinalCameraPosition();
	}

	void setBlitzCameraHandlerPositionX(double tX)
	{
		gBlitzCameraHandlerData.mBaseCameraPosition.x = tX;
		gBlitzCameraHandlerData.mBaseCameraPosition = clampPositionToGeoRectangle(gBlitzCameraHandlerData.mBaseCameraPosition, gBlitzCameraHandlerData.mCameraRange);
		calculateAndSetFinalCameraPosition();
	}

	void setBlitzCameraHandlerPositionY(double tY)
	{
		gBlitzCameraHandlerData.mBaseCameraPosition.y = tY;
		gBlitzCameraHandlerData.mBaseCameraPosition = clampPositionToGeoRectangle(gBlitzCameraHandlerData.mBaseCameraPosition, gBlitzCameraHandlerData.mCameraRange);
		calculateAndSetFinalCameraPosition();
	}

	Vector3D* getBlitzCameraHandlerScaleReference()
	{
		return &gBlitzCameraHandlerData.mScale;
	}

	Vector3D getBlitzCameraHandlerScale()
	{
		return gBlitzCameraHandlerData.mScale;
	}

	void setBlitzCameraHandlerScale2D(double tScale)
	{
		gBlitzCameraHandlerData.mScale = Vector3D(tScale, tScale, 1);
	}

	void setBlitzCameraHandlerScaleX(double tScaleX)
	{
		gBlitzCameraHandlerData.mScale.x = tScaleX;
	}

	void setBlitzCameraHandlerScaleY(double tScaleY)
	{
		gBlitzCameraHandlerData.mScale.y = tScaleY;
	}

	double* getBlitzCameraHandlerRotationZReference()
	{
		return &gBlitzCameraHandlerData.mAngle;
	}

	double getBlitzCameraHandlerRotationZ()
	{
		return gBlitzCameraHandlerData.mAngle;
	}

	void setBlitzCameraHandlerRotationZ(double tAngle)
	{
		gBlitzCameraHandlerData.mAngle = tAngle;
	}

	Position2D* getBlitzCameraHandlerEffectPositionReference()
	{
		return &gBlitzCameraHandlerData.mEffectOffset;
	}

	void setBlitzCameraHandlerEffectPositionOffset(const Position2D& tPosition)
	{
		gBlitzCameraHandlerData.mEffectOffset = tPosition;
	}

	int getBlitzCameraHandlerEntityID()
	{
		if (!isBlitzCameraHandlerEnabled()) {
			logWarning("Requestion disabled camera handler entity ID.");
		}

		return -10;
	}

	const GeoRectangle2D& getBlitzCameraHandlerRange()
	{
		return gBlitzCameraHandlerData.mCameraRange;
	}

	void setBlitzCameraHandlerRange(const GeoRectangle2D& tRectangle)
	{
		const auto sz = getScreenSize();
		auto finalRectangle = tRectangle;
		finalRectangle.mBottomRight = finalRectangle.mBottomRight - Vector2D(sz.x, sz.y);
		gBlitzCameraHandlerData.mCameraRange = finalRectangle;
	}

	void setBlitzCameraPositionBasedOnCenterPoint(const Position& tCenter)
	{
		const auto sz = getScreenSize();
		const auto topLeft = vecSub(tCenter, Vector3D((sz.x / 2) * gBlitzCameraHandlerData.mScale.x, (sz.y / 2) * gBlitzCameraHandlerData.mScale.y, 0));
		setBlitzCameraHandlerPosition(topLeft);
	}

	void setBlitzCameraScreenShake(int tDuration, double tFrequency, int tAmplitude, double tPhaseOffset)
	{
		gBlitzCameraHandlerData.mScreenShakeDurationLeft = tDuration;
		gBlitzCameraHandlerData.mScreenShakeFrequency = tFrequency;
		gBlitzCameraHandlerData.mScreenShakeAmplitude = tAmplitude;
		gBlitzCameraHandlerData.mScreenShakePhaseOffset = tPhaseOffset;
	}

	void setBlitzCameraScreenShakeDefault() {
		setBlitzCameraScreenShake(10, 60.0, -4, 0.0);
	}

	void setBlitzCameraZoom(const Vector2D& tPosition, double tZoomFactor)
	{
		setBlitzCameraHandlerEffectPositionOffset(tPosition);
		setBlitzCameraHandlerScale2D(tZoomFactor);
	}
	void setBlitzCameraZoom(const GeoRectangle2D& tZoomArea) {
		const auto sz = getScreenSize();
		const auto center = (tZoomArea.mTopLeft + tZoomArea.mBottomRight) / 2.0;
		setBlitzCameraHandlerEffectPositionOffset(center);
		setBlitzCameraHandlerScaleX(tZoomArea.mBottomRight.x / sz.x);
		setBlitzCameraHandlerScaleY(tZoomArea.mBottomRight.y / sz.y);
	}
}