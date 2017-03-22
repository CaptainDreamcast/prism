#include "../include/input.h"

#include <kos.h>

#include "../include/log.h"

static maple_device_t* gCont;
static cont_state_t* gState;

void updateInput() {
  if ((gCont = maple_enum_dev(0, 0)) != NULL) {
    gState = (cont_state_t *) maple_dev_status(gCont);
  } else {
    gState = (cont_state_t*)0;
  }
}

void resetInput() {
	hasPressedAFlank();
	hasPressedBFlank();
	hasPressedXFlank();
	hasPressedYFlank();
	hasPressedLFlank();
	hasPressedRFlank();
	hasPressedLeftFlank();
	hasPressedRightFlank();
	hasPressedUpFlank();
	hasPressedDownFlank();
	hasPressedStartFlank();
	hasPressedAbortFlank();
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

int hasPressedL() {
  if (!gState)
    return 0;

  return (gState->ltrig >= 64);
}

int hasPressedR() {
  if (!gState)
    return 0;

  return (gState->rtrig >= 64);
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

static int hasPressedFlank(int tCurrent, int* tFlank) {
  int returnValue = 0;

  debugInteger(tCurrent);
  debugInteger((*tFlank));
  if (tCurrent && !(*tFlank)) {
    returnValue = 1;
  }

  (*tFlank) = tCurrent;
  return returnValue;
}

static int gAFlank = 0;
int hasPressedAFlank() {
  return hasPressedFlank(hasPressedA(), &gAFlank);
}

static int gBFlank = 0;
int hasPressedBFlank() {
  return hasPressedFlank(hasPressedB(), &gBFlank);
}

static int gXFlank = 0;
int hasPressedXFlank() {
  return hasPressedFlank(hasPressedX(), &gXFlank);
}

static int gYFlank = 0;
int hasPressedYFlank() {
  return hasPressedFlank(hasPressedY(), &gYFlank);
}

static int gLeftFlank = 0;
int hasPressedLeftFlank() {
  return hasPressedFlank(hasPressedLeft(), &gLeftFlank);
}

static int gRightFlank = 0;
int hasPressedRightFlank() {
  return hasPressedFlank(hasPressedRight(), &gRightFlank);
}

static int gUpFlank = 0;
int hasPressedUpFlank() {
  return hasPressedFlank(hasPressedUp(), &gUpFlank);
}

static int gDownFlank = 0;
int hasPressedDownFlank() {
  return hasPressedFlank(hasPressedDown(), &gDownFlank);
}

static int gLFlank = 0;
int hasPressedLFlank() {
  return hasPressedFlank(hasPressedL(), &gLFlank);
}

static int gRFlank = 0;
int hasPressedRFlank() {
  return hasPressedFlank(hasPressedR(), &gRFlank);
}

static int gStartFlank = 0;
int hasPressedStartFlank() {
  return hasPressedFlank(hasPressedStart(), &gStartFlank);
}

static int gAbortFlank = 0;
int hasPressedAbortFlank() {
  debugLog("check abort flank");
  return hasPressedFlank(hasPressedAbort(), &gAbortFlank);
}
