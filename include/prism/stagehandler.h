#pragma once

#include "animation.h"
#include "tweening.h"
#include "actorhandler.h"

namespace prism {

ActorBlueprint getStageHandler();

void setStageHandlerNoDelayedLoading();
void setStageHandlerAccelerationPhysics();
void setStageHandlerTweening();
void setStageCameraRange(const GeoRectangle2D& tRange);

int addScrollingBackground(float tScrollingFactor, float tZ);
int addScrollingBackgroundWithMovementIn2D(float tDeltaX, float tDeltaY, float tZ);
int addBackgroundElement(int tBackgroundID, const Position& tPosition, char* tPath, const Animation& tAnimation);
int addBackgroundElementWithTextureData(int tBackgroundID, const Position& tPosition, TextureData* tTextureData, const Animation& tAnimation);
TextureData* getBackgroundElementTextureData(int tBackgroundID, int tElementID);

Position getRealScreenPosition(int tBackgroundID, const Position& tPos);
void scrollBackgroundRight(float tAccel);
void scrollBackgroundDown(float tAccel);

Position* getScrollingBackgroundPositionReference(int tID);
void setScrollingBackgroundPosition(int tID, const Position& tPos);
void setScrollingBackgroundMaxVelocity(int tID, float tVel);
PhysicsObject* getScrollingBackgroundPhysics(int tID);
void setScrollingBackgroundPhysics(int tID, const PhysicsObject& tPhysics);
void setScrollingBackgroundInvisible(int tID);
void setScrollingBackgroundVisible(int tID);

void addStageHandlerScreenShake(float tStrength);
void setStageHandlerMaximumScreenShake(float tStrength);

void loadStageFromScript(const char* tPath);

void imguiStageHandler();

}