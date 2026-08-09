#include "prism/screeneffect.h"

#include "prism/file.h"
#include "prism/timer.h"
#include "prism/memoryhandler.h"
#include "prism/physicshandler.h"
#include "prism/system.h"
#include "prism/texture.h"
#include "prism/system.h"
#include "prism/log.h"
#include "prism/math.h"
#include "prism/stlutil.h"
#include "prism/tweening.h"

#ifdef _WIN32
#include <imgui/imgui.h>
#include "prism/windows/debugimgui_win.h"
#endif

using namespace std;

namespace prism {

	struct FadeInStruct;

	typedef int(*IsScreenEffectOverFunction)(struct FadeInStruct*);

	typedef struct FadeInStruct {
		AnimationHandlerElement** mAnimationElements;
		int mAnimationAmount;

		PhysicsHandlerElement* mPhysicsElement;
		Vector3D* mSize;

		PhysicsHandlerElement* mAlphaPhysicsElement;
		float* mAlpha;

		Duration mDuration;

		ScreenEffectFinishedCB mCB;
		void* mCaller;

		IsScreenEffectOverFunction mIsOverFunction;
	} FadeIn;

	typedef struct {
		float mR;
		float mG;
		float mB;

	} FadeColor;

	using namespace std;

	static struct {
		TextureData mWhiteTexture;
		int mIsActive;
		float mZ;

		int mFullLineSize;

		AnimationHandlerElement* mScreenFillElement;

		FadeColor mFadeColor;

		map<int, FadeIn> mFadeIns;

		AnimationHandlerElement* mTintElement;
		FadeColor mTintColor;
		float mTintAlpha;
		int mTintTweenID;
		float mTintZ;
	} gScreenEffect;

#ifdef _WIN32
	static void imguiFadeIns() {
		if (ImGui::TreeNode("Fade Ins"))
		{
			ImGui::Text("Count: %d", (int)gScreenEffect.mFadeIns.size());
			for (auto& e : gScreenEffect.mFadeIns)
			{
				ImGui::PushID(e.first);
				if (ImGui::TreeNode("Fade In"))
				{
					ImGui::Text("ID: %d", e.first);
					ImGui::Text("Duration: %.0f", e.second.mDuration);
					ImGui::Text("Size: %f, %f", e.second.mSize->x, e.second.mSize->y);
					ImGui::Text("Alpha: %f", *e.second.mAlpha);
					ImGui::Text("Amount: %d", e.second.mAnimationAmount);
					ImGui::Text("Is Over: %d", e.second.mIsOverFunction(&e.second));
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}

	static void imguiScreenEffectsData()
	{
		ImGui::Text("Active: %d", gScreenEffect.mIsActive);
		ImGui::Text("Z: %f", gScreenEffect.mZ);
		ImGui::Text("Full Line Size: %d", gScreenEffect.mFullLineSize);
		ImGui::Text("Fade Color: %f, %f, %f", gScreenEffect.mFadeColor.mR, gScreenEffect.mFadeColor.mG, gScreenEffect.mFadeColor.mB);
		imguiFadeIns();
	}

	void imguiScreenEffects() {
		static bool isWindowShown = false;
		imguiPrismAddTab("Prism", "ScreenEffects", &isWindowShown);
		if (isWindowShown)
		{
			ImGui::Begin("ScreenEffects", &isWindowShown);
			imguiScreenEffectsData();
			ImGui::End();
		}
	}
#endif

	void initScreenEffects() {
		gScreenEffect.mWhiteTexture = createWhiteTexture();
		gScreenEffect.mFullLineSize = 10;
		gScreenEffect.mZ = 80;
		gScreenEffect.mScreenFillElement = NULL;
		gScreenEffect.mFadeColor.mR = gScreenEffect.mFadeColor.mG = gScreenEffect.mFadeColor.mB = 0;

		gScreenEffect.mTintElement = NULL;
		gScreenEffect.mTintColor.mR = gScreenEffect.mTintColor.mG = gScreenEffect.mTintColor.mB = 0;
		gScreenEffect.mTintAlpha = 0;
		gScreenEffect.mTintTweenID = -1;
		gScreenEffect.mTintZ = gScreenEffect.mZ - 1;

		gScreenEffect.mIsActive = 1;
	}

	void shutdownScreenEffects() {
		if (!gScreenEffect.mIsActive) return;

		unloadTexture(gScreenEffect.mWhiteTexture);
		gScreenEffect.mIsActive = 0;
	}

	static void resetTintState() {
		gScreenEffect.mTintElement = NULL; // let animation handler clean this properly
		gScreenEffect.mTintAlpha = 0;
		gScreenEffect.mTintTweenID = -1;
	}

	static void loadScreenEffectHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		gScreenEffect.mFadeIns.clear();
		resetTintState();
	}

	static void unloadScreenEffectHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		gScreenEffect.mFadeIns.clear();
		resetTintState();
	}

	static void unloadedBehaviour(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller) {
		if (tOptionalCB != NULL) {
			addTimerCB(tDuration, tOptionalCB, tCaller);
		}
	}

	static int isVerticalLineFadeInOver(FadeIn* tFadeIn) {
		return tFadeIn->mSize->y <= 0;
	}

	static void removeFadeIn(FadeIn* e) {
		removeFromPhysicsHandler(e->mPhysicsElement);
		removeFromPhysicsHandler(e->mAlphaPhysicsElement);

		int i;
		for (i = 0; i < e->mAnimationAmount; i++) {
			removeHandledAnimation(e->mAnimationElements[i]);
		}

		freeMemory(e->mAnimationElements);
	}

	static void updateSingleFadeInAnimation(FadeIn* e, int i) {
		setAnimationSize(e->mAnimationElements[i], *e->mSize, Vector3D(0, 0, 0));
		setAnimationTransparency(e->mAnimationElements[i], *e->mAlpha);
	}

	static int updateFadeIn(FadeIn& e) {

		if (e.mIsOverFunction(&e)) {
			if (e.mCB) e.mCB(e.mCaller);
			removeFadeIn(&e);
			return 1;
		}

		int i;
		for (i = 0; i < e.mAnimationAmount; i++) {
			updateSingleFadeInAnimation(&e, i);
		}

		return 0;
	}

	static void applyTint();

	static void updateScreenEffectHandler(void* tData) {
		(void)tData;
		setProfilingSectionMarkerCurrentFunction();
		stl_int_map_remove_predicate(gScreenEffect.mFadeIns, updateFadeIn);

		if (gScreenEffect.mTintTweenID != -1 && gScreenEffect.mTintElement) {
			applyTint();
		}
	}

	static void addFadeIn_internal(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller, const Vector3D& tStartPatchSize, const Vector3D& tFullPatchSize, const Vector3D& tSizeDelta, float tStartAlpha, float tAlphaDelta, IsScreenEffectOverFunction tIsOverFunc) {
		if (!gScreenEffect.mIsActive) {
			unloadedBehaviour(tDuration, tOptionalCB, tCaller);
			return;
		}

		ScreenSize screen = getScreenSize();

		FadeIn e;
		e.mDuration = tDuration;
		e.mCB = tOptionalCB;
		e.mCaller = tCaller;
		e.mDuration = tDuration;

		e.mPhysicsElement = addToPhysicsHandler(tStartPatchSize);
		addAccelerationToHandledPhysics(e.mPhysicsElement, tSizeDelta);
		e.mSize = &getPhysicsFromHandler(e.mPhysicsElement)->mPosition;

		e.mAlphaPhysicsElement = addToPhysicsHandler(Vector3D(tStartAlpha, 0, 0));
		addAccelerationToHandledPhysics(e.mAlphaPhysicsElement, Vector3D(tAlphaDelta, 0, 0));
		e.mAlpha = &getPhysicsFromHandler(e.mAlphaPhysicsElement)->mPosition.x;

		e.mIsOverFunction = tIsOverFunc;

		int amountX = (int)((screen.x + (tFullPatchSize.x - 1)) / tFullPatchSize.x);
		int amountY = (int)((screen.y + (tFullPatchSize.y - 1)) / tFullPatchSize.y);
		e.mAnimationAmount = amountX * amountY;

		e.mAnimationElements = (AnimationHandlerElement**)allocMemory(e.mAnimationAmount * sizeof(AnimationHandlerElement*));
		Position p = Vector3D(0, 0, gScreenEffect.mZ);
		int i;
		for (i = 0; i < e.mAnimationAmount; i++) {
			e.mAnimationElements[i] = playAnimationLoop(p, &gScreenEffect.mWhiteTexture, createOneFrameAnimation(), makeRectangleFromTexture(gScreenEffect.mWhiteTexture));
			updateSingleFadeInAnimation(&e, i);
			setAnimationColor(e.mAnimationElements[i], gScreenEffect.mFadeColor.mR, gScreenEffect.mFadeColor.mG, gScreenEffect.mFadeColor.mB);

			p = vecAdd(p, Vector3D(tFullPatchSize.x, 0, 0));
			if (p.x >= screen.x) {
				p = vecAdd(p, tFullPatchSize);
				p.x = 0;
			}
		}

		stl_int_map_push_back(gScreenEffect.mFadeIns, e);
	}

	static int isFadeInOver(FadeIn* e) {
		return (*e->mAlpha) <= 0;
	}

	void addFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller) {
		float da = -1 / (float)tDuration;
		Vector3D patchSize = Vector3D(getScreenSize().x, getScreenSize().y, 1);
		addFadeIn_internal(tDuration, tOptionalCB, tCaller, patchSize, patchSize, Vector3D(0, 0, 0), 1, da, isFadeInOver);
	}

	void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller) {
		float dy = -gScreenEffect.mFullLineSize / (float)tDuration;
		addFadeIn_internal(tDuration, tOptionalCB, tCaller, Vector3D(getScreenSize().x, gScreenEffect.mFullLineSize + 1, 1), Vector3D(getScreenSize().x, gScreenEffect.mFullLineSize, 1), Vector3D(0, dy, 0), 1, 0, isVerticalLineFadeInOver);
	}

	static int skipSingleFadeInCB(FadeIn& e) {
		if (e.mCB) e.mCB(e.mCaller);
		removeFadeIn(&e);
		return 1;

	}

	void skipFadeIn()
	{
		if (!gScreenEffect.mIsActive) return;
		stl_int_map_remove_predicate(gScreenEffect.mFadeIns, skipSingleFadeInCB);
	}

	typedef struct {
		void* mCaller;
		ScreenEffectFinishedCB mCB;
	} FadeOutData;

	static void fadeOutOverCB(void* tCaller) {
		FadeOutData* e = (FadeOutData*)tCaller;
		setScreenBlack();
		if (e->mCB) {
			e->mCB(e->mCaller);
		}
		freeMemory(e);
	}

	static int isFadeOutOver(FadeIn* e) {
		return (*e->mAlpha) >= 1;
	}

	void addFadeOut(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller) {
		auto da = 1 / (float)tDuration;
		Vector3D patchSize = Vector3D(getScreenSize().x, getScreenSize().y, 1);
		FadeOutData* e = (FadeOutData*)allocMemory(sizeof(FadeOutData));
		e->mCB = tOptionalCB;
		e->mCaller = tCaller;

		addFadeIn_internal(tDuration, fadeOutOverCB, e, patchSize, patchSize, Vector3D(0, 0, 0), 0, da, isFadeOutOver);
	}

	void setFadeColor(Color tColor) {
		getRGBFromColor(tColor, &gScreenEffect.mFadeColor.mR, &gScreenEffect.mFadeColor.mG, &gScreenEffect.mFadeColor.mB);
	}

	void setFadeColorRGB(float r, float g, float b) {
		gScreenEffect.mFadeColor.mR = r;
		gScreenEffect.mFadeColor.mG = g;
		gScreenEffect.mFadeColor.mB = b;
	}

	void setScreenEffectZ(float tZ)
	{
		gScreenEffect.mZ = tZ;
	}

	static void ensureTintElement() {
		if (gScreenEffect.mTintElement) return;

		ScreenSize screen = getScreenSize();
		Position p = Vector3D(0, 0, gScreenEffect.mTintZ);
		gScreenEffect.mTintElement = playAnimationLoop(p, &gScreenEffect.mWhiteTexture, createOneFrameAnimation(), makeRectangleFromTexture(gScreenEffect.mWhiteTexture));
		setAnimationSize(gScreenEffect.mTintElement, Vector3D(screen.x, screen.y, 1), Vector3D(0, 0, 0));
	}

	static void applyTint() {
		if (!gScreenEffect.mTintElement) return;
		setAnimationColor(gScreenEffect.mTintElement, gScreenEffect.mTintColor.mR, gScreenEffect.mTintColor.mG, gScreenEffect.mTintColor.mB);
		setAnimationTransparency(gScreenEffect.mTintElement, gScreenEffect.mTintAlpha);
	}

	static void cancelTintTween() {
		if (gScreenEffect.mTintTweenID != -1) {
			removeTween(gScreenEffect.mTintTweenID);
			gScreenEffect.mTintTweenID = -1;
		}
	}

	void removeScreenTint() {
		cancelTintTween();
		gScreenEffect.mTintAlpha = 0;
		if (gScreenEffect.mTintElement) {
			removeHandledAnimation(gScreenEffect.mTintElement);
			gScreenEffect.mTintElement = NULL;
		}
	}

	void setScreenTintRGBA(float tR, float tG, float tB, float tAlpha) {
		if (!gScreenEffect.mIsActive) return;

		cancelTintTween();
		gScreenEffect.mTintColor.mR = tR;
		gScreenEffect.mTintColor.mG = tG;
		gScreenEffect.mTintColor.mB = tB;
		gScreenEffect.mTintAlpha = tAlpha;

		if (tAlpha <= 0) {
			removeScreenTint();
			return;
		}

		ensureTintElement();
		applyTint();
	}

	void setScreenTint(Color tColor, float tAlpha) {
		float r, g, b;
		getRGBFromColor(tColor, &r, &g, &b);
		setScreenTintRGBA(r, g, b, tAlpha);
	}

	static void tintTweenOverCB(void* tCaller) {
		(void)tCaller;
		gScreenEffect.mTintTweenID = -1;
		if (gScreenEffect.mTintAlpha <= 0) {
			removeScreenTint();
		}
	}

	void tweenScreenTintRGBA(float tR, float tG, float tB, float tStartAlpha, float tEndAlpha, Duration tDuration, TweeningFunction tFunc) {
		if (!gScreenEffect.mIsActive) return;

		cancelTintTween();
		gScreenEffect.mTintColor.mR = tR;
		gScreenEffect.mTintColor.mG = tG;
		gScreenEffect.mTintColor.mB = tB;
		gScreenEffect.mTintAlpha = tStartAlpha;

		ensureTintElement();
		applyTint();

		gScreenEffect.mTintTweenID = tweenDouble(&gScreenEffect.mTintAlpha, tStartAlpha, tEndAlpha, tFunc, tDuration, tintTweenOverCB, NULL);
	}

	void tweenScreenTint(Color tColor, float tStartAlpha, float tEndAlpha, Duration tDuration, TweeningFunction tFunc) {
		float r, g, b;
		getRGBFromColor(tColor, &r, &g, &b);
		tweenScreenTintRGBA(r, g, b, tStartAlpha, tEndAlpha, tDuration, tFunc);
	}

	void setScreenTintZ(float tZ) {
		gScreenEffect.mTintZ = tZ;
		if (gScreenEffect.mTintElement) {
			setAnimationPosition(gScreenEffect.mTintElement, Vector3D(0, 0, tZ));
		}
	}

	void addDamageFlashColored(Color tColor, float tStartAlpha, Duration tDuration) {
		tweenScreenTint(tColor, tStartAlpha, 0, tDuration, inverseQuadraticTweeningFunction);
	}

	void addDamageFlash(Duration tDuration) {
		addDamageFlashColored(COLOR_RED, 0.5, tDuration);
	}

	void drawColoredRectangle(const GeoRectangle& tRect, Color tColor) {
		if (!gScreenEffect.mIsActive) return;

		float dx = (tRect.mBottomRight.x - tRect.mTopLeft.x);
		float dy = (tRect.mBottomRight.y - tRect.mTopLeft.y);
		dx /= gScreenEffect.mWhiteTexture.mTextureSize.x;
		dy /= gScreenEffect.mWhiteTexture.mTextureSize.y;

		scaleDrawing3D(Vector3D(dx, dy, 1), tRect.mTopLeft);
		setDrawingBaseColor(tColor);
		drawSprite(gScreenEffect.mWhiteTexture, tRect.mTopLeft, makeRectangleFromTexture(gScreenEffect.mWhiteTexture));
		setDrawingParametersToIdentity();
	}

	void drawColoredHorizontalLine(const Position& tA, const Position& tB, Color tColor)
	{
		if (tA.y != tB.y) return;

		float x = min(tA.x, tB.x);
		auto w = (float)abs((float)(tB.x - tA.x));
		drawColoredRectangle(GeoRectangle(x, tA.y, tA.z, w, 1), tColor);
	}

	void drawColoredPoint(const Position& tPoint, Color tColor) {
		drawColoredRectangle(GeoRectangle(tPoint.x, tPoint.y, tPoint.z, 1, 1), tColor);
	}

	void setScreenBlack() {
		if (!gScreenEffect.mIsActive) return;

		setScreenColor(COLOR_BLACK);
	}

	void unsetScreenBlack() {
		if (!gScreenEffect.mIsActive) return;

		unsetScreenColor();
	}

	void setScreenWhite() {
		setScreenColor(COLOR_WHITE);
	}

	void unsetScreenWhite() {
		unsetScreenColor();
	}

	TextureData getEmptyWhiteTexture()
	{
		return gScreenEffect.mWhiteTexture;
	}

	TextureData* getEmptyWhiteTextureReference()
	{
		return &gScreenEffect.mWhiteTexture;
	}

	ActorBlueprint getScreenEffectHandler()
	{
		return makeActorBlueprint(loadScreenEffectHandler, unloadScreenEffectHandler, updateScreenEffectHandler);
	}

}