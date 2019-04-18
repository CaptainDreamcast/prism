#include "prism/input.h"

#include <kos.h>

#include "prism/log.h"
#include "prism/screeneffect.h"

typedef struct {
	maple_device_t* mCont;
	cont_state_t* mState;
} Controller;

typedef struct {
	uint8_t mPreviousStates[256];	
	uint8_t mCurrentStates[256];
	int mPreviousModifiers;
	int mCurrentModifiers;

	int mIsActive;
} Keyboard;

typedef struct {
	int mIsActive;
	int mHasPolledGun;
	int mGunIsFired;
	int mHasDisplayedWhiteLongEnough;
} LightGun;

static struct {
	Controller mControllers[MAXIMUM_CONTROLLER_AMOUNT];
	LightGun mLightGuns[MAXIMUM_CONTROLLER_AMOUNT];
	Keyboard mKeyboards[MAXIMUM_CONTROLLER_AMOUNT];
} gData;

void initInput() {}

static void updateSingleLightGun(int i) {
	gData.mLightGuns[i].mIsActive = 1;

	if(hasPressedASingle(i)) {
		maple_gun_enable(gData.mControllers[i].mCont->port);
		if(!gData.mLightGuns[i].mHasPolledGun) {
			setScreenWhite();
			gData.mLightGuns[i].mHasPolledGun = 1;	
		} else if(!gData.mLightGuns[i].mGunIsFired){
			gData.mLightGuns[i].mGunIsFired = 1;
		}
		else if(!gData.mLightGuns[i].mHasDisplayedWhiteLongEnough){
			gData.mLightGuns[i].mHasDisplayedWhiteLongEnough = 1;
			unsetScreenWhite();
		}
	} else {
		gData.mLightGuns[i].mHasPolledGun = 0;
		gData.mLightGuns[i].mGunIsFired = 0;
		gData.mLightGuns[i].mHasDisplayedWhiteLongEnough = 0;
		unsetScreenColor();
	}
}

static void disableSingleLightGun(int i) {
		gData.mLightGuns[i].mIsActive = 0;
		gData.mLightGuns[i].mHasPolledGun = 0;
		gData.mLightGuns[i].mGunIsFired = 0;
		gData.mLightGuns[i].mHasDisplayedWhiteLongEnough = 0;
		unsetScreenColor();
}

static void updateSingleKeyboard(int i) {
	kbd_state_t* kbd = (kbd_state_t*)gData.mControllers[i].mState;
	memcpy(gData.mKeyboards[i].mPreviousStates, gData.mKeyboards[i].mCurrentStates, 256);	
	memcpy(gData.mKeyboards[i].mCurrentStates, kbd->matrix, 256);
	gData.mKeyboards[i].mPreviousModifiers = gData.mKeyboards[i].mCurrentModifiers;
	gData.mKeyboards[i].mCurrentModifiers = kbd->shift_keys;

	gData.mKeyboards[i].mIsActive = 1;
}

static void disableSingleKeyboard(int i) {
	gData.mKeyboards[i].mIsActive = 0;
}

static void updateSingleInput(int i) {
	  if ((gData.mControllers[i].mCont = maple_enum_dev(i, 0)) != NULL) {
	    gData.mControllers[i].mState = (cont_state_t *) maple_dev_status(gData.mControllers[i].mCont);
	    if(gData.mControllers[i].mCont->info.functions & MAPLE_FUNC_LIGHTGUN)  {
		updateSingleLightGun(i);
	    } else if(gData.mLightGuns[i].mIsActive) {
		disableSingleLightGun(i);
        }

		if(gData.mControllers[i].mCont->info.functions & MAPLE_FUNC_KEYBOARD)  {
			updateSingleKeyboard(i);
	    } else if(gData.mKeyboards[i].mIsActive) {
			disableSingleKeyboard(i);
		}
		
	  } else {
	    gData.mControllers[i].mState = (cont_state_t*)0;
	  }

}

void updateInputPlatform() {
	int i;
	for(i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		updateSingleInput(i);
	}
}


static int evaluateDreamcastButtonA(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->buttons & CONT_A);
}

static int evaluateDreamcastButtonB(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->buttons & CONT_B);
}

static int evaluateDreamcastButtonX(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->buttons & CONT_X);
}

static int evaluateDreamcastButtonY(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->buttons & CONT_Y);
}

static int evaluateDreamcastButtonL(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->ltrig >= 64);
}

static int evaluateDreamcastButtonR(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->rtrig >= 64);
}

static int evaluateDreamcastButtonLeft(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return ((gData.mControllers[i].mState->buttons & CONT_DPAD_LEFT) || (gData.mControllers[i].mState->joyx <= -64));
}

static int evaluateDreamcastButtonRight(int i) {
  if (!gData.mControllers[i].mState) return 0;
  return ((gData.mControllers[i].mState->buttons & CONT_DPAD_RIGHT) || (gData.mControllers[i].mState->joyx >= 64));
}

static int evaluateDreamcastButtonUp(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return ((gData.mControllers[i].mState->buttons & CONT_DPAD_UP) || (gData.mControllers[i].mState->joyy <= -64));
}

static int evaluateDreamcastButtonDown(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return ((gData.mControllers[i].mState->buttons & CONT_DPAD_DOWN) || (gData.mControllers[i].mState->joyy >= 64));
}

static int evaluateDreamcastButtonStart(int i) {
	if (!gData.mControllers[i].mState) return 0;
	return (gData.mControllers[i].mState->buttons & CONT_START);
}

typedef int(*InputEvaluationFunction)(int);

static InputEvaluationFunction gDreamcastButtonMapping[] = {
	evaluateDreamcastButtonA,
	evaluateDreamcastButtonB,
	evaluateDreamcastButtonX,
	evaluateDreamcastButtonY,
	evaluateDreamcastButtonL,
	evaluateDreamcastButtonR,
	evaluateDreamcastButtonLeft,
	evaluateDreamcastButtonRight,
	evaluateDreamcastButtonUp,
	evaluateDreamcastButtonDown,
	evaluateDreamcastButtonStart,
};

static ControllerButtonPrism gButtonMapping[MAXIMUM_CONTROLLER_AMOUNT][CONTROLLER_BUTTON_AMOUNT_PRISM] = {
	{
		CONTROLLER_A_PRISM,
		CONTROLLER_B_PRISM,
		CONTROLLER_X_PRISM,
		CONTROLLER_Y_PRISM,
		CONTROLLER_L_PRISM,
		CONTROLLER_R_PRISM,
		CONTROLLER_LEFT_PRISM,
		CONTROLLER_RIGHT_PRISM,
		CONTROLLER_UP_PRISM,
		CONTROLLER_DOWN_PRISM,
		CONTROLLER_START_PRISM,
	},
	{
		CONTROLLER_A_PRISM,
		CONTROLLER_B_PRISM,
		CONTROLLER_X_PRISM,
		CONTROLLER_Y_PRISM,
		CONTROLLER_L_PRISM,
		CONTROLLER_R_PRISM,
		CONTROLLER_LEFT_PRISM,
		CONTROLLER_RIGHT_PRISM,
		CONTROLLER_UP_PRISM,
		CONTROLLER_DOWN_PRISM,
		CONTROLLER_START_PRISM,
	},
};

int hasPressedASingle(int i) {
  return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_A_PRISM]](i);
}

int hasPressedBSingle(int i) {
  return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_B_PRISM]](i);
}

int hasPressedXSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_X_PRISM]](i);
}

int hasPressedYSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_Y_PRISM]](i);
}

int hasPressedLeftSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_LEFT_PRISM]](i);
}

int hasPressedRightSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_RIGHT_PRISM]](i);
}

int hasPressedUpSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_UP_PRISM]](i);
}

int hasPressedDownSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_DOWN_PRISM]](i);
}

int hasPressedLSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_L_PRISM]](i);
}

int hasPressedRSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_R_PRISM]](i);
}

int hasPressedStartSingle(int i) {
	return gDreamcastButtonMapping[gButtonMapping[i][CONTROLLER_START_PRISM]](i);
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

int hasShotGunSingle(int i)  {
	return gData.mLightGuns[i].mGunIsFired;
}

Vector3D getShotPositionSingle(int i) {
	(void)i; // TODO: light gun multiplayer
	if(!gData.mLightGuns[i].mIsActive) return makePosition(0,0,0);
	int x, y;
	maple_gun_read_pos(&x, &y);
	return makePosition(x, y, 0);
}

void forceMouseCursorToWindow() {}
void releaseMouseCursorFromWindow() {}

int isUsingControllerSingle(int i) {
	(void)i;
	return 1;
}


void addControllerRumbleSingle(int i, Duration tDuration, int tFrequency, double tAmplitude) {
	(void)i;
	(void)tDuration;
	(void)tFrequency;
	(void)tAmplitude;
	// TODO
}


void turnControllerRumbleOffSingle(int i) {
	(void)i;
	// TODO
}

int hasPressedRawButton(int i, ControllerButtonPrism tButton) {
	return gDreamcastButtonMapping[tButton](i);
}

static int gKOSKeyMapping[] = {
	KBD_KEY_A,
	KBD_KEY_B,
	KBD_KEY_C,
	KBD_KEY_D,
	KBD_KEY_E,
	KBD_KEY_F,
	KBD_KEY_G,
	KBD_KEY_H,
	KBD_KEY_I,
	KBD_KEY_J,
	KBD_KEY_K,
	KBD_KEY_L,
	KBD_KEY_M,
	KBD_KEY_N,
	KBD_KEY_O,
	KBD_KEY_P,
	KBD_KEY_Q,
	KBD_KEY_R,
	KBD_KEY_S,
	KBD_KEY_T,
	KBD_KEY_U,
	KBD_KEY_V,
	KBD_KEY_W,
	KBD_KEY_X,
	KBD_KEY_Y,
	KBD_KEY_Z,
	KBD_KEY_0,
	KBD_KEY_1,
	KBD_KEY_2,
	KBD_KEY_3,
	KBD_KEY_4,
	KBD_KEY_5,
	KBD_KEY_6,
	KBD_KEY_7,
	KBD_KEY_8,
	KBD_KEY_9,
	KBD_KEY_SPACE,
	KBD_KEY_LEFT,
	KBD_KEY_RIGHT,
	KBD_KEY_UP,
	KBD_KEY_DOWN,
	KBD_KEY_F1,
	KBD_KEY_F2,
	KBD_KEY_F3,
	KBD_KEY_F4,
	KBD_KEY_F5,
	KBD_KEY_F6,
	KBD_KEY_SCRLOCK,
	KBD_KEY_PAUSE,
	53,
	KBD_MOD_LCTRL, // CTRL LEFT with modifier keys
	KBD_MOD_LSHIFT,
	KBD_KEY_ENTER,
	KBD_KEY_BACKSPACE,
	KBD_KEY_DEL,
	KBD_KEY_PERIOD,
	KBD_KEY_SLASH,
};

static int hasPressedKeyboardKeyInternal(KeyboardKeyPrism tKey, uint8_t* tStates, int tModifiers) {
	switch(tKey) {
		case KEYBOARD_CTRL_LEFT_PRISM:
		case KEYBOARD_SHIFT_LEFT_PRISM:
			return tModifiers & gKOSKeyMapping[tKey];
		default:
			return tStates[gKOSKeyMapping[tKey]];
	}
}

int hasPressedRawKeyboardKey(KeyboardKeyPrism tKey) {
	if(!gData.mKeyboards[1].mIsActive) return 0;

	return hasPressedKeyboardKeyInternal(tKey, gData.mKeyboards[1].mCurrentStates, gData.mKeyboards[1].mCurrentModifiers);
}

int hasPressedKeyboardKeyFlank(KeyboardKeyPrism tKey) {
	if(!gData.mKeyboards[1].mIsActive) return 0;

	int previous = hasPressedKeyboardKeyInternal(tKey, gData.mKeyboards[1].mPreviousStates, gData.mKeyboards[1].mPreviousModifiers);
	int current = hasPressedKeyboardKeyInternal(tKey, gData.mKeyboards[1].mCurrentStates, gData.mKeyboards[1].mCurrentModifiers);

	return !previous && current;
}

int hasPressedKeyboardMultipleKeyFlank(int tKeyAmount, ...) {
	if (!tKeyAmount) return 0;
	if(!gData.mKeyboards[1].mIsActive) return 0;

	int i;
	va_list vl;
	va_start(vl, tKeyAmount);

	int previousKeyPressed = 1, currentKeyPressed = 1;
	for (i = 0; i < tKeyAmount; i++)
	{
		KeyboardKeyPrism singleKey = (KeyboardKeyPrism)va_arg(vl, int);
		int previousSingle = hasPressedKeyboardKeyInternal(singleKey, gData.mKeyboards[1].mPreviousStates, gData.mKeyboards[1].mPreviousModifiers);
		int currentSingle = hasPressedKeyboardKeyInternal(singleKey, gData.mKeyboards[1].mCurrentStates, gData.mKeyboards[1].mCurrentModifiers);

		previousKeyPressed = previousKeyPressed && previousSingle;
		currentKeyPressed = currentKeyPressed && currentSingle;
	}
	va_end(vl);

	return !previousKeyPressed && currentKeyPressed;
}

ControllerButtonPrism getButtonForController(int i, ControllerButtonPrism tTargetButton)
{
	return gButtonMapping[i][tTargetButton];
}

void setButtonForController(int i, ControllerButtonPrism tTargetButton, ControllerButtonPrism tButtonValue)
{
	gButtonMapping[i][tTargetButton] = tButtonValue;
}

KeyboardKeyPrism getButtonForKeyboard(int i, ControllerButtonPrism tTargetButton)
{
	(void)i;
	(void)tTargetButton;
	return (KeyboardKeyPrism)0; // TODO
}

void setButtonForKeyboard(int i, ControllerButtonPrism tTargetButton, KeyboardKeyPrism tKeyValue)
{
	(void)i;
	(void)tTargetButton;
	(void)tKeyValue;
	// TODO
}

