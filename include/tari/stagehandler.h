#ifndef TARI_STAGEHANDLER_H
#define TARI_STAGEHANDLER_H

#include "animation.h"
#include "tweening.h"

fup void setupStageHandler();
fup void shutdownStageHandler();
fup void updateStageHandler();
fup void setStageHandlerNoDelayedLoading();
fup void setStageHandlerAccelerationPhysics();
fup void setStageHandlerTweening();
fup void setStageCameraRange(GeoRectangle tRange);

fup int addScrollingBackground(double tScrollingFactor, double tZ);
fup int addBackgroundElement(int tBackgroundID, Position tPosition, char* tPath, Animation tAnimation);
fup TextureData* getBackgroundElementTextureData(int tBackgroundID, int tElementID);

fup Position getRealScreenPosition(int tBackgroundID, Position tPos);
fup void scrollBackgroundRight(double tAccel);
fup void scrollBackgroundDown(double tAccel);

fup Position* getScrollingBackgroundPositionReference(int tID);
fup void setScrollingBackgroundPosition(int tID, Position tPos);
fup void setScrollingBackgroundMaxVelocity(int tID, double tVel);
fup PhysicsObject* getScrollingBackgroundPhysics(int tID);
fup void setScrollingBackgroundPhysics(int tID, PhysicsObject tPhysics);
fup void setScrollingBackgroundInvisible(int tID);
fup void setScrollingBackgroundVisible(int tID);

fup void addStageHandlerScreenShake(double tStrength);
fup void setStageHandlerMaximumScreenShake(double tStrength);

fup void loadStageFromScript(char* tPath);

#endif
