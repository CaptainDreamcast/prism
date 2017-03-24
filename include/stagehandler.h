#ifndef TARI_STAGEHANDLER_H
#define TARI_STAGEHANDLER_H

#include "animation.h"

fup void setupStageHandler();
fup void shutdownStageHandler();
fup void updateStageHandler();


fup int addScrollingBackground(double tScrollingFactor, double tZ);
fup int addBackgroundElement(int tBackgroundID, Position tPosition, char* tPath, Animation tAnimation);
fup Position getRealScreenPosition(int tBackgroundID, Position tPos);
fup void scrollBackgroundRight(double tAccel);
fup Position* getScrollingBackgroundPositionReference(int tID);


#endif
