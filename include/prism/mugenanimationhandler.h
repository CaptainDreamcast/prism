#pragma once 

#include <list>

#include "actorhandler.h"
#include "geometry.h"
#include "mugenspritefilereader.h"
#include "mugenanimationreader.h"
#include "collision.h"

namespace prism {

typedef int MugenDuration;
struct CollisionListData;
struct CollisionListElement;

typedef struct {
	CollisionListData* mList;
	CollisionListElement* mElement;
	Collider mCollider;
} MugenAnimationHandlerHitboxElement;

struct MugenAnimationHandlerElement {

	int mID;
	MugenAnimation* mAnimation;
	MugenSpriteFile* mSprites;
	int mStep;

	int mIsFacingRight;
	int mIsFacingDown;

	MugenDuration mOverallTime;
	MugenDuration mStepTime;
	MugenSpriteFileSprite* mSprite;
	int mHasSprite;

	int mHasPassiveHitCB;
	void* mPassiveHitCaller;
	void(*mPassiveHitCB)(void* tCaller, void* tCollisionData, int tOtherCollisionList);

	int mHasAttackHitCB;
	void* mAttackHitCaller;
	void(*mAttackHitCB)(void* tCaller, void* tCollisionData, int tOtherCollisionList);

	int mHasAnimationFinishedCallback;
	void* mAnimationFinishedCaller;
	void(*mAnimationFinishedCB)(void* tCaller);

	int mHasPassiveHitboxes;
	CollisionListData* mPassiveCollisionList;
	void* mPassiveCollisionData;

	int mHasAttackHitboxes;
	CollisionListData* mAttackCollisionList;
	void* mAttackCollisionData;

	std::list<MugenAnimationHandlerHitboxElement> mActiveHitboxes;

	Position mPlayerPositionReference;

	float mDrawScale;

	Vector2D mBaseDrawScale;

	Position mOffset;

	int mHasRectangleWidth;
	int mRectangleWidth;

	int mHasRectangleHeight;
	int mRectangleHeight;

	int mHasCameraPositionReference;
	Position* mCameraPositionReference;

	int mHasCameraScaleReference;
	Vector3D* mCameraScaleReference;
	float mCameraScaleFactor;

	int mHasCameraAngleReference;
	float* mCameraAngleReference;

	int mHasCameraEffectPositionReference;
	Position2D* mCameraEffectPositionReference;

	int mIsInvisible;
	int mIsColorSolid;
	int mIsColorInverted;

	float mBaseDrawAngle;

	int mHasBasePositionReference;
	Position* mBasePositionReference;

	int mHasScaleReference;
	Vector3D* mScaleReference;

	int mHasAngleReference;
	float* mAngleReference;

	int mHasBlendType;
	BlendType mBlendType;

	int mHasConstraintRectangle;
	GeoRectangle2D mConstraintRectangle;

	int mIsPaused;
	int mIsLooping;
	int mHasLooped;

	float mTimeDilatationNow;
	float mTimeDilatation;

	float mOffsetR;
	float mOffsetG;
	float mOffsetB;
	float mR;
	float mG;
	float mB;
	float mAlpha;
	float mDestinationAlpha;
	float mColorFactor;

	int mHasShear;
	float mShearLowerScaleDeltaX;
	float mShearLowerOffsetX;
	int mIsSpriteOffsetForcedToCenter;

	Vector2D mCoordinateSystemScale;

	int mIsCollisionDebugActive;
};

void setMugenAnimationHandlerPixelCenter(const Vector2D& tPixelCenter);

MugenAnimationHandlerElement* addMugenAnimation(MugenAnimation* tStartAnimation, MugenSpriteFile* tSprites, const Position& tPosition);
void removeMugenAnimation(MugenAnimationHandlerElement* tElement);
int isRegisteredMugenAnimation(MugenAnimationHandlerElement* tElement);

void setMugenAnimationBaseDrawScale(MugenAnimationHandlerElement* tElement, float tScale);
void setMugenAnimationBasePosition(MugenAnimationHandlerElement* tElement, Position* tPosition);
void setMugenAnimationScaleReference(MugenAnimationHandlerElement* tElement, Vector3D* tScale);
void setMugenAnimationAngleReference(MugenAnimationHandlerElement* tElement, float* tAngle);

void setMugenAnimationCollisionActive(MugenAnimationHandlerElement* tElement, CollisionListData* tCollisionList, void(*tFunc)(void*, void*, int), void* tCaller, void* tCollisionData);
void setMugenAnimationPassiveCollisionActive(MugenAnimationHandlerElement* tElement, CollisionListData* tCollisionList, void(*tFunc)(void*, void*, int), void* tCaller, void* tCollisionData);
void setMugenAnimationAttackCollisionActive(MugenAnimationHandlerElement* tElement, CollisionListData* tCollisionList, void(*tFunc)(void*, void*, int), void* tCaller, void* tCollisionData);

void setMugenAnimationNoLoop(MugenAnimationHandlerElement* tElement);
void setMugenAnimationCallback(MugenAnimationHandlerElement* tElement, void(*tFunc)(void*), void* tCaller);

int getMugenAnimationAnimationNumber(MugenAnimationHandlerElement* tElement);
int getMugenAnimationAnimationStep(MugenAnimationHandlerElement* tElement);
int getMugenAnimationAnimationStepAmount(MugenAnimationHandlerElement* tElement);
int getMugenAnimationAnimationStepDuration(MugenAnimationHandlerElement* tElement);
int getMugenAnimationRemainingAnimationTime(MugenAnimationHandlerElement* tElement);
int hasMugenAnimationLooped(MugenAnimationHandlerElement* tElement);
int getMugenAnimationTime(MugenAnimationHandlerElement* tElement);
int getMugenAnimationDuration(MugenAnimationHandlerElement* tElement);
int isMugenAnimationDurationInfinite(MugenAnimationHandlerElement* tElement);
Vector3DI getMugenAnimationSprite(MugenAnimationHandlerElement* tElement);
void setMugenAnimationFaceDirection(MugenAnimationHandlerElement* tElement, int tIsFacingRight);
void setMugenAnimationVerticalFaceDirection(MugenAnimationHandlerElement* tElement, int tIsFacingDown);
void setMugenAnimationRectangleWidth(MugenAnimationHandlerElement* tElement, int tWidth);
void setMugenAnimationRectangleHeight(MugenAnimationHandlerElement* tElement, int tHeight);
void setMugenAnimationCameraPositionReference(MugenAnimationHandlerElement* tElement, Position* tCameraPosition);
void removeMugenAnimationCameraPositionReference(MugenAnimationHandlerElement* tElement);
void setMugenAnimationCameraScaleReference(MugenAnimationHandlerElement* tElement, Position* tCameraScale);
void removeMugenAnimationCameraScaleReference(MugenAnimationHandlerElement* tElement);
void setMugenAnimationCameraScaleFactor(MugenAnimationHandlerElement* tElement, float tScaleFactor);
void setMugenAnimationCameraAngleReference(MugenAnimationHandlerElement* tElement, float* tCameraAngle);
void removeMugenAnimationCameraAngleReference(MugenAnimationHandlerElement* tElement);
void setMugenAnimationCameraEffectPositionReference(MugenAnimationHandlerElement* tElement, Position2D* tCameraEffectPosition);
void removeMugenAnimationCameraEffectPositionReference(MugenAnimationHandlerElement* tElement);

void setMugenAnimationInvisible(MugenAnimationHandlerElement* tElement);
void setMugenAnimationVisibility(MugenAnimationHandlerElement* tElement, int tIsVisible);
void setMugenAnimationDrawScale(MugenAnimationHandlerElement* tElement, const Vector2D& tScale);
void setMugenAnimationDrawSize(MugenAnimationHandlerElement* tElement, const Vector2D& tSize);
void setMugenAnimationDrawAngle(MugenAnimationHandlerElement* tElement, float tAngle);
void setMugenAnimationColorOffset(MugenAnimationHandlerElement* tElement, float tR, float tG, float tB);
void setMugenAnimationColor(MugenAnimationHandlerElement* tElement, float tR, float tG, float tB);
void setMugenAnimationColorSolid(MugenAnimationHandlerElement* tElement, float tR, float tG, float tB);
void setMugenAnimationTransparency(MugenAnimationHandlerElement* tElement, float tOpacity);
void setMugenAnimationDestinationTransparency(MugenAnimationHandlerElement* tElement, float tOpacity);
void setMugenAnimationPosition(MugenAnimationHandlerElement* tElement, const Position& tPosition);
void setMugenAnimationPositionX(MugenAnimationHandlerElement* tElement, float tX);
void setMugenAnimationPositionY(MugenAnimationHandlerElement* tElement, float tY);
void setMugenAnimationBlendType(MugenAnimationHandlerElement* tElement, BlendType tBlendType);
void setMugenAnimationSprites(MugenAnimationHandlerElement* tElement, MugenSpriteFile* tSprites);
void setMugenAnimationConstraintRectangle(MugenAnimationHandlerElement* tElement, const GeoRectangle2D& tConstraintRectangle);

void setMugenAnimationSpeed(MugenAnimationHandlerElement* tElement, float tSpeed);

Position getMugenAnimationPosition(MugenAnimationHandlerElement* tElement);
int getMugenAnimationScreenBoundingBox(MugenAnimationHandlerElement* tElement, GeoRectangle2D* oBoundingBox);
int getMugenAnimationIsFacingRight(MugenAnimationHandlerElement* tElement);
int getMugenAnimationIsFacingDown(MugenAnimationHandlerElement* tElement);
int getMugenAnimationVisibility(MugenAnimationHandlerElement* tElement);
Vector2D getMugenAnimationDrawScale(MugenAnimationHandlerElement* tElement);
BlendType getMugenAnimationBlendType(MugenAnimationHandlerElement* tElement);
float getMugenAnimationTransparency(MugenAnimationHandlerElement* tElement);

float getMugenAnimationDrawAngle(MugenAnimationHandlerElement* tElement);
float getMugenAnimationColorRed(MugenAnimationHandlerElement* tElement);
float getMugenAnimationColorGreen(MugenAnimationHandlerElement* tElement);
float getMugenAnimationColorBlue(MugenAnimationHandlerElement* tElement);

Position* getMugenAnimationBasePosition(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationColorRedReference(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationColorGreenReference(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationColorBlueReference(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationTransparencyReference(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationScaleXReference(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationScaleYReference(MugenAnimationHandlerElement* tElement);
float* getMugenAnimationBaseScaleReference(MugenAnimationHandlerElement* tElement);
Position* getMugenAnimationPositionReference(MugenAnimationHandlerElement* tElement);

void setMugenAnimationAnimationStepDuration(MugenAnimationHandlerElement* tElement, int tDuration);

void changeMugenAnimation(MugenAnimationHandlerElement* tElement, MugenAnimation* tNewAnimation);
void changeMugenAnimationWithStartStep(MugenAnimationHandlerElement* tElement, MugenAnimation* tNewAnimation, int tStartStep);

int isStartingMugenAnimationElementWithID(MugenAnimationHandlerElement* tElement, int tStepID);
int getTimeFromMugenAnimationElement(MugenAnimationHandlerElement* tElement, int tStep);
int getMugenAnimationElementFromTimeOffset(MugenAnimationHandlerElement* tElement, int tTime);
int isMugenAnimationTimeOffsetInAnimation(MugenAnimationHandlerElement* tElement, int tTime);
int getMugenAnimationTimeWhenStepStarts(MugenAnimationHandlerElement* tElement, int tStep);
int getMugenAnimationIsLooping(MugenAnimationHandlerElement* tElement);

void advanceMugenAnimationOneTick(MugenAnimationHandlerElement* tElement);

int hasMugenAnimationChanged(MugenAnimationHandlerElement* tElement, const MugenAnimationHandlerElement& tCompareData);
MugenAnimationHandlerElement saveMugenAnimation(MugenAnimationHandlerElement* tElement);
void restoreMugenAnimation(MugenAnimationHandlerElement* tElement, const MugenAnimationHandlerElement& tRestorationData);

void setMugenAnimationCollisionDebug(MugenAnimationHandlerElement* tElement, int tIsActive);

void pauseMugenAnimation(MugenAnimationHandlerElement* tElement);
void unpauseMugenAnimation(MugenAnimationHandlerElement* tElement);

void setMugenAnimationColorFactor(MugenAnimationHandlerElement* tElement, float tColorFactor);
void setMugenAnimationColorInverted(MugenAnimationHandlerElement* tElement, int tIsInverted);

float getMugenAnimationShearLowerOffsetX(MugenAnimationHandlerElement* tElement);
void setMugenAnimationShearX(MugenAnimationHandlerElement* tElement, float tLowerScaleDeltaX, float tLowerOffsetX);
void setMugenAnimationCoordinateSystemScale(MugenAnimationHandlerElement* tElement, const Vector2D& tCoordinateSystemScale);
void setMugenAnimationIsSpriteOffsetForcedToCenter(MugenAnimationHandlerElement* tElement, int tIsSpriteOffsetForcedToCenter);

std::list<MugenAnimationHandlerHitboxElement>& getMugenAnimationActiveHitboxes(MugenAnimationHandlerElement* tElement);

void resetMugenAnimation(MugenAnimationHandlerElement* tElement);

void pauseMugenAnimationHandler();
void unpauseMugenAnimationHandler();

ActorBlueprint getMugenAnimationHandler();

void imguiMugenAnimationHandler();

}
