#ifndef TARI_STAGEHANDLER_H
#define TARI_STAGEHANDLER_H

#include "animation.h"

fup void setupStageHandler();
fup void shutdownStageHandler();
fup void updateStageHandler();
fup void setStageHandlerNoDelayedLoading();

fup int addScrollingBackground(double tScrollingFactor, double tZ);
fup int addBackgroundElement(int tBackgroundID, Position tPosition, char* tPath, Animation tAnimation);

fup Position getRealScreenPosition(int tBackgroundID, Position tPos);
fup void scrollBackgroundRight(double tAccel);

fup Position* getScrollingBackgroundPositionReference(int tID);
fup void setScrollingBackgroundPosition(int tID, Position tPos);
fup void setScrollingBackgroundMaxVelocity(int tID, double tVel);
fup PhysicsObject* getScrollingBackgroundPhysics(int tID);
fup void setScrollingBackgroundPhysics(int tID, PhysicsObject tPhysics);

fup void loadStageFromScript(char* tPath);

#endif
