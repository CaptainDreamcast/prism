#pragma once

#define MAXIMUM_CONTROLLER_AMOUNT 2

#include "geometry.h"

fup void updateInput();
fup void resetInput();

fup int hasPressedA();
fup int hasPressedB();
fup int hasPressedX();
fup int hasPressedY();
fup int hasPressedLeft();
fup int hasPressedRight();
fup int hasPressedUp();
fup int hasPressedDown();
fup int hasPressedL();
fup int hasPressedR();
fup int hasPressedStart();
fup int hasPressedAbort();

fup int hasPressedAFlank();
fup int hasPressedBFlank();
fup int hasPressedXFlank();
fup int hasPressedYFlank();
fup int hasPressedLeftFlank();
fup int hasPressedRightFlank();
fup int hasPressedUpFlank();
fup int hasPressedDownFlank();
fup int hasPressedLFlank();
fup int hasPressedRFlank();
fup int hasPressedStartFlank();
fup int hasPressedAbortFlank();

fup double getLeftStickNormalizedX();
fup double getLeftStickNormalizedY();
fup double getLNormalized();
fup double getRNormalized();

fup int hasShotGun();
fup int hasShotGunFlank();
fup Vector3D getShotPosition();

fup int hasPressedASingle(int i);
fup int hasPressedBSingle(int i);
fup int hasPressedXSingle(int i);
fup int hasPressedYSingle(int i);
fup int hasPressedLeftSingle(int i);
fup int hasPressedRightSingle(int i);
fup int hasPressedUpSingle(int i);
fup int hasPressedDownSingle(int i);
fup int hasPressedLSingle(int i);
fup int hasPressedRSingle(int i);
fup int hasPressedStartSingle(int i);
fup int hasPressedAbortSingle(int i);

fup int hasPressedAFlankSingle(int i);
fup int hasPressedBFlankSingle(int i);
fup int hasPressedXFlankSingle(int i);
fup int hasPressedYFlankSingle(int i);
fup int hasPressedLeftFlankSingle(int i);
fup int hasPressedRightFlankSingle(int i);
fup int hasPressedUpFlankSingle(int i);
fup int hasPressedDownFlankSingle(int i);
fup int hasPressedLFlankSingle(int i);
fup int hasPressedRFlankSingle(int i);
fup int hasPressedStartFlankSingle(int i);
fup int hasPressedAbortFlankSingle(int i);

fup double getSingleLeftStickNormalizedX(int i);
fup double getSingleLeftStickNormalizedY(int i);
fup double getSingleLNormalized(int i);
fup double getSingleRNormalized(int i);

fup int hasShotGunSingle(int i);
fup int hasShotGunFlankSingle(int i);
fup Vector3D getShotPositionSingle(int i);

fup void setMainController(int i);
fup int getMainController();

void forceMouseCursorToWindow();
void releaseMouseCursorFromWindow();

int isUsingControllerSingle(int i);
int isUsingController();

double getFishingRodIntensity();
double getFishingRodIntensitySingle(int i);
