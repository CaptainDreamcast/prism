#include "include/input.h"

#include <kos.h>

#include "include/log.h"

maple_device_t* gCont;
cont_state_t* gState;

void updateInput() {
  if ((gCont = maple_enum_dev(0, 0)) != NULL) {
    gState = (cont_state_t *) maple_dev_status(gCont);
  } else {
    gState = (cont_state_t*)0;
  }
}

int hasPressedA() {
  if (!gState)
    return 0;

  return (gState->buttons & CONT_A);
}

int hasPressedB() {
  if (!gState)
    return 0;

  return (gState->buttons & CONT_B);
}

int hasPressedX() {
  if (!gState)
    return 0;

  return (gState->buttons & CONT_X);
}

int hasPressedY() {
  if (!gState)
    return 0;

  return (gState->buttons & CONT_Y);
}

int hasPressedLeft() {
  if (!gState)
    return 0;

  return ((gState->buttons & CONT_DPAD_LEFT) || (gState->joyx <= -64));
}

int hasPressedRight() {
  if (!gState)
    return 0;

  return ((gState->buttons & CONT_DPAD_RIGHT) || (gState->joyx >= 64));
}

int hasPressedUp() {
  if (!gState)
    return 0;

  return ((gState->buttons & CONT_DPAD_UP) || (gState->joyy <= -64));
}

int hasPressedDown() {
  if (!gState)
    return 0;

  return ((gState->buttons & CONT_DPAD_DOWN) || (gState->joyy >= 64));
}

int hasPressedStart() {
  if (!gState)
    return 0;

  return (gState->buttons & CONT_START);
}

int hasPressedAbort() {
  if (!gState)
    return 0;

  uint32_t exitCode = (CONT_A | CONT_B | CONT_X | CONT_Y | CONT_START);
  return ((gState->buttons & exitCode) == exitCode);
}

int hasPressedFlank(int tCurrent, int* tFlank) {
  int returnValue = 0;

  debugInteger(tCurrent);
  debugInteger((*tFlank));
  if (tCurrent && !(*tFlank)) {
    returnValue = 1;
  }

  (*tFlank) = tCurrent;
  return returnValue;
}

int gAFlank = 0;
int hasPressedAFlank() {
  return hasPressedFlank(hasPressedA(), &gAFlank);
}

int gBFlank = 0;
int hasPressedBFlank() {
  return hasPressedFlank(hasPressedB(), &gBFlank);
}

int gXFlank = 0;
int hasPressedXFlank() {
  return hasPressedFlank(hasPressedX(), &gXFlank);
}

int gYFlank = 0;
int hasPressedYFlank() {
  return hasPressedFlank(hasPressedY(), &gYFlank);
}

int gLeftFlank = 0;
int hasPressedLeftFlank() {
  return hasPressedFlank(hasPressedLeft(), &gLeftFlank);
}

int gRightFlank = 0;
int hasPressedRightFlank() {
  return hasPressedFlank(hasPressedRight(), &gRightFlank);
}

int gUpFlank = 0;
int hasPressedUpFlank() {
  return hasPressedFlank(hasPressedUp(), &gUpFlank);
}

int gDownFlank = 0;
int hasPressedDownFlank() {
  return hasPressedFlank(hasPressedDown(), &gDownFlank);
}

int gStartFlank = 0;
int hasPressedStartFlank() {
  return hasPressedFlank(hasPressedStart(), &gStartFlank);
}

int gAbortFlank = 0;
int hasPressedAbortFlank() {
  debugLog("check abort flank");
  return hasPressedFlank(hasPressedAbort(), &gAbortFlank);
}
