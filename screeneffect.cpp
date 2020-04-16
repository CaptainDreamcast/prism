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

using namespace std;

struct FadeInStruct;

typedef int(*IsScreenEffectOverFunction)(struct FadeInStruct* );

typedef struct FadeInStruct {
	AnimationHandlerElement** mAnimationElements;
	int mAnimationAmount;

	PhysicsHandlerElement* mPhysicsElement;
	Vector3D* mSize;

	PhysicsHandlerElement* mAlphaPhysicsElement;
	double* mAlpha;

	Duration mDuration;

	ScreenEffectFinishedCB mCB;
	void* mCaller;

	IsScreenEffectOverFunction mIsOverFunction;
} FadeIn;

typedef struct {
	double mR;
	double mG;
	double mB;

} FadeColor;

using namespace std;

static struct {
	TextureData mWhiteTexture;
	int mIsActive;
	double mZ;

	int mFullLineSize;

	AnimationHandlerElement* mScreenFillElement;

	FadeColor mFadeColor;

	map<int, FadeIn> mFadeIns;
} gScreenEffect;

void initScreenEffects() {
	gScreenEffect.mWhiteTexture = createWhiteTexture();
	gScreenEffect.mFullLineSize = 10;
	gScreenEffect.mZ = 80;
	gScreenEffect.mScreenFillElement = NULL;
	gScreenEffect.mFadeColor.mR = gScreenEffect.mFadeColor.mG = gScreenEffect.mFadeColor.mB = 0;

	gScreenEffect.mIsActive = 1;
}

void shutdownScreenEffects() {
	if (!gScreenEffect.mIsActive) return;

	unloadTexture(gScreenEffect.mWhiteTexture);
	gScreenEffect.mIsActive = 0;
}

static void loadScreenEffectHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gScreenEffect.mFadeIns.clear();
}

static void unloadScreenEffectHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gScreenEffect.mFadeIns.clear();
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
	setAnimationSize(e->mAnimationElements[i], *e->mSize, makePosition(0, 0, 0));
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

static void updateScreenEffectHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_remove_predicate(gScreenEffect.mFadeIns, updateFadeIn);
}

static void addFadeIn_internal(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller, Vector3D tStartPatchSize, Vector3D tFullPatchSize, Vector3D tSizeDelta, double tStartAlpha, double tAlphaDelta, IsScreenEffectOverFunction tIsOverFunc) {
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

	e.mAlphaPhysicsElement = addToPhysicsHandler(makePosition(tStartAlpha, 0, 0));
	addAccelerationToHandledPhysics(e.mAlphaPhysicsElement, makePosition(tAlphaDelta, 0, 0));
	e.mAlpha = &getPhysicsFromHandler(e.mAlphaPhysicsElement)->mPosition.x;

	e.mIsOverFunction = tIsOverFunc;

	int amountX = (int)((screen.x + (tFullPatchSize.x - 1)) / tFullPatchSize.x);
	int amountY = (int)((screen.y + (tFullPatchSize.y - 1)) / tFullPatchSize.y);
	e.mAnimationAmount = amountX*amountY;

	e.mAnimationElements = (AnimationHandlerElement**)allocMemory(e.mAnimationAmount*sizeof(AnimationHandlerElement*));
	Position p = makePosition(0, 0, gScreenEffect.mZ);
	int i;
	for (i = 0; i < e.mAnimationAmount; i++) {
		e.mAnimationElements[i] = playAnimationLoop(p, &gScreenEffect.mWhiteTexture, createOneFrameAnimation(), makeRectangleFromTexture(gScreenEffect.mWhiteTexture));
		updateSingleFadeInAnimation(&e, i);
		setAnimationColor(e.mAnimationElements[i], gScreenEffect.mFadeColor.mR, gScreenEffect.mFadeColor.mG, gScreenEffect.mFadeColor.mB);

		p = vecAdd(p, makePosition(tFullPatchSize.x, 0, 0));
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
	double da = -1 / (double)tDuration;
	Vector3D patchSize = makePosition(getScreenSize().x, getScreenSize().y, 1);
	addFadeIn_internal(tDuration, tOptionalCB, tCaller, patchSize, patchSize, makePosition(0, 0, 0), 1, da, isFadeInOver);
}

void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller) {
	double dy = -gScreenEffect.mFullLineSize / (double)tDuration;
	addFadeIn_internal(tDuration, tOptionalCB, tCaller, makePosition(getScreenSize().x, gScreenEffect.mFullLineSize+1, 1), makePosition(getScreenSize().x, gScreenEffect.mFullLineSize, 1), makePosition(0, dy, 0), 1, 0, isVerticalLineFadeInOver);
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
	double da = 1 / (double)tDuration;
	Vector3D patchSize = makePosition(getScreenSize().x, getScreenSize().y, 1);
	FadeOutData* e = (FadeOutData*)allocMemory(sizeof(FadeOutData));
	e->mCB = tOptionalCB;
	e->mCaller = tCaller;

	addFadeIn_internal(tDuration, fadeOutOverCB, e, patchSize, patchSize, makePosition(0, 0, 0), 0, da, isFadeOutOver);
}

void setFadeColor(Color tColor) {
	getRGBFromColor(tColor, &gScreenEffect.mFadeColor.mR, &gScreenEffect.mFadeColor.mG, &gScreenEffect.mFadeColor.mB);
}

void setFadeColorRGB(double r, double g, double b) {
	gScreenEffect.mFadeColor.mR = r;
	gScreenEffect.mFadeColor.mG = g;
	gScreenEffect.mFadeColor.mB = b;
}

void setScreenEffectZ(double tZ)
{
	gScreenEffect.mZ = tZ;
}

void drawColoredRectangle(GeoRectangle tRect, Color tColor) {
	if (!gScreenEffect.mIsActive) return;

	double dx = (tRect.mBottomRight.x - tRect.mTopLeft.x);
	double dy = (tRect.mBottomRight.y - tRect.mTopLeft.y);
	dx /= gScreenEffect.mWhiteTexture.mTextureSize.x;
	dy /= gScreenEffect.mWhiteTexture.mTextureSize.y;

	scaleDrawing3D(makePosition(dx, dy, 1), tRect.mTopLeft);
	setDrawingBaseColor(tColor);
	drawSprite(gScreenEffect.mWhiteTexture, tRect.mTopLeft, makeRectangleFromTexture(gScreenEffect.mWhiteTexture));
	setDrawingParametersToIdentity();
}

void drawColoredHorizontalLine(Position tA, Position tB, Color tColor)
{
	if (tA.y != tB.y) return;

	double x = min(tA.x, tB.x);
	double w = (double)abs((double)(tB.x - tA.x));
	drawColoredRectangle(makeGeoRectangle3D(x, tA.y, tA.z, w, 1), tColor);
}

void drawColoredPoint(Position tPoint, Color tColor) {
	drawColoredRectangle(makeGeoRectangle3D(tPoint.x, tPoint.y, tPoint.z, 1, 1), tColor);
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

