#pragma once

#include "animation.h"
#include "tweening.h"
#include "actorhandler.h"

ActorBlueprint getStageHandler();

void setStageHandlerNoDelayedLoading();
void setStageHandlerAccelerationPhysics();
void setStageHandlerTweening();
void setStageCameraRange(const GeoRectangle2D& tRange);

int addScrollingBackground(double tScrollingFactor, double tZ);
int addScrollingBackgroundWithMovementIn2D(double tDeltaX, double tDeltaY, double tZ);
int addBackgroundElement(int tBackgroundID, const Position& tPosition, char* tPath, const Animation& tAnimation);
int addBackgroundElementWithTextureData(int tBackgroundID, const Position& tPosition, TextureData* tTextureData, const Animation& tAnimation);
TextureData* getBackgroundElementTextureData(int tBackgroundID, int tElementID);

Position getRealScreenPosition(int tBackgroundID, const Position& tPos);
void scrollBackgroundRight(double tAccel);
void scrollBackgroundDown(double tAccel);

Position* getScrollingBackgroundPositionReference(int tID);
void setScrollingBackgroundPosition(int tID, const Position& tPos);
void setScrollingBackgroundMaxVelocity(int tID, double tVel);
PhysicsObject* getScrollingBackgroundPhysics(int tID);
void setScrollingBackgroundPhysics(int tID, const PhysicsObject& tPhysics);
void setScrollingBackgroundInvisible(int tID);
void setScrollingBackgroundVisible(int tID);

void addStageHandlerScreenShake(double tStrength);
void setStageHandlerMaximumScreenShake(double tStrength);

void loadStageFromScript(const char* tPath);
