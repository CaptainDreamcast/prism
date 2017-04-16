#ifndef INPUT_TARI
#define INPUT_TARI

#include "common/header.h"

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

#endif
