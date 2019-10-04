#pragma once

#include "animation.h"
#include "tweening.h"
#include "actorhandler.h"

ActorBlueprint getStageHandler();

void setStageHandlerNoDelayedLoading();
void setStageHandlerAccelerationPhysics();
void setStageHandlerTweening();
void setStageCameraRange(GeoRectangle tRange);

int addScrollingBackground(double tScrollingFactor, double tZ);
int addScrollingBackgroundWithMovementIn2D(double tDeltaX, double tDeltaY, double tZ);
int addBackgroundElement(int tBackgroundID, Position tPosition, char* tPath, Animation tAnimation);
int addBackgroundElementWithTextureData(int tBackgroundID, Position tPosition, TextureData* tTextureData, Animation tAnimation);
TextureData* getBackgroundElementTextureData(int tBackgroundID, int tElementID);

Position getRealScreenPosition(int tBackgroundID, Position tPos);
void scrollBackgroundRight(double tAccel);
void scrollBackgroundDown(double tAccel);

Position* getScrollingBackgroundPositionReference(int tID);
void setScrollingBackgroundPosition(int tID, Position tPos);
void setScrollingBackgroundMaxVelocity(int tID, double tVel);
PhysicsObject* getScrollingBackgroundPhysics(int tID);
void setScrollingBackgroundPhysics(int tID, PhysicsObject tPhysics);
void setScrollingBackgroundInvisible(int tID);
void setScrollingBackgroundVisible(int tID);

void addStageHandlerScreenShake(double tStrength);
void setStageHandlerMaximumScreenShake(double tStrength);

void loadStageFromScript(char* tPath);
