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
	return hasPressedA() && hasPressedB() && hasPressedX() && hasPressedY() && hasPressedStart();
}