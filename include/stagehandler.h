#ifndef TARI_STAGEHANDLER_H
#define TARI_STAGEHANDLER_H

#include "animation.h"

void setupStageHandler();
void shutdownStageHandler();
void updateStageHandler();


int addScrollingBackground(double tScrollingFactor, double tZ);
int addBackgroundElement(int tBackgroundID, Position tPosition, char* tPath, Animation tAnimation);
Position getRealScreenPosition(int tBackgroundID, Position tPos);
void scrollBackgroundRight(double tAccel);
Position* getScrollingBackgroundPositionReference(int tID);


#endif
