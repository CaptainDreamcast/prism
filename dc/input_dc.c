#include "../include/input.h"

#include <kos.h>

#include "../include/log.h"

typedef struct {
	maple_device_t* mCont;
	cont_state_t* mState;
} Controller;

static struct {
	Controller mControllers[MAXIMUM_CONTROLLER_AMOUNT];
} gData;

static void updateSingleInput(int i) {
	  if ((gData.mControllers[i].mCont = maple_enum_dev(i, 0)) != NULL) {
	    gData.mControllers[i].mState = (cont_state_t *) maple_dev_status(gData.mControllers[i].mCont);
	  } else {
	    gData.mControllers[i].mState = (cont_state_t*)0;
	  }
}

void updateInput() {
	int i;
	for(i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		updateSingleInput(i);
	}
}

int hasPressedASingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return (gData.mControllers[i].mState->buttons & CONT_A);
}

int hasPressedBSingle(int i) {
  if (!gData.mControllers[i].mState) 
	return 0;

  return (gData.mControllers[i].mState->buttons & CONT_B);
}

int hasPressedXSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return (gData.mControllers[i].mState->buttons & CONT_X);
}

int hasPressedYSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return (gData.mControllers[i].mState->buttons & CONT_Y);
}

int hasPressedLeftSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return ((gData.mControllers[i].mState->buttons & CONT_DPAD_LEFT) || (gData.mControllers[i].mState->joyx <= -64));
}

int hasPressedRightSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return ((gData.mControllers[i].mState->buttons & CONT_DPAD_RIGHT) || (gData.mControllers[i].mState->joyx >= 64));
}

int hasPressedUpSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return ((gData.mControllers[i].mState->buttons & CONT_DPAD_UP) || (gData.mControllers[i].mState->joyy <= -64));
}

int hasPressedDownSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return ((gData.mControllers[i].mState->buttons & CONT_DPAD_DOWN) || (gData.mControllers[i].mState->joyy >= 64));
}

int hasPressedLSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return (gData.mControllers[i].mState->ltrig >= 64);
}

int hasPressedRSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return (gData.mControllers[i].mState->rtrig >= 64);
}

int hasPressedStartSingle(int i) {
  if (!gData.mControllers[i].mState)
    return 0;

  return (gData.mControllers[i].mState->buttons & CONT_START);
}

int hasPressedAbortSingle(int i) {
	return hasPressedASingle(i) && hasPressedBSingle(i) && hasPressedXSingle(i) && hasPressedYSingle(i) && hasPressedStartSingle(i);
}


double getSingleLeftStickNormalizedX(int i) {
	if (!gData.mControllers[i].mState) return 0;

	return gData.mControllers[i].mState->joyx / 128.0;
}
double getSingleLeftStickNormalizedY(int i) {
	if (!gData.mControllers[i].mState) return 0;

	return gData.mControllers[i].mState->joyy / 128.0;
}
double getSingleLNormalized(int i) {
	if (!gData.mControllers[i].mState) return 0;

	return gData.mControllers[i].mState->ltrig / 128.0;
}
double getSingleRNormalized(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return gData.mControllers[i].mState->rtrig / 128.0;
}
