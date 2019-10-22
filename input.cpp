#include "prism/input.h"

#include "prism/log.h"
#include "prism/math.h"

typedef struct {
	uint8_t mPrev[CONTROLLER_BUTTON_AMOUNT_PRISM];
	uint8_t mCurrent[CONTROLLER_BUTTON_AMOUNT_PRISM];
	uint8_t mAbortPrev = 0;
	uint8_t mAbortCurrent = 0;
	uint8_t mShotPrev = 0;
	uint8_t mShotCurrent = 0;
} InputStatus;

typedef struct {
	int mIsActive;
	ControllerButtonPrism mTargetButton;
	int mIsSetting;
	int mIsSettingController;
	int mFlankCheckDone;
	void(*mSettingOptionalCB)(void*); 
	void(*mKeyboardWaitingCB)(void*, KeyboardKeyPrism);
	void(*mControllerWaitingCB)(void*, ControllerButtonPrism);
	void* mCaller;
} SetInputData;

static struct {
	InputStatus mStatus[MAXIMUM_CONTROLLER_AMOUNT];
	SetInputData mSetInput[MAXIMUM_CONTROLLER_AMOUNT];

	int mMainController;
} gPrismGeneralInputData;

void resetInputForAllControllers() {
	int i;
	for (i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		gPrismGeneralInputData.mSetInput[i].mIsActive = 0;
	}
}

int hasPressedAFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_A_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_A_PRISM];
}

int hasPressedBFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_B_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_B_PRISM];
}
int hasPressedXFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_X_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_X_PRISM];
}

int hasPressedYFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_Y_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_Y_PRISM];
}

int hasPressedLeftFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_LEFT_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_LEFT_PRISM];
}

int hasPressedRightFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_RIGHT_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_RIGHT_PRISM];
}

int hasPressedUpFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_UP_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_UP_PRISM];
}

int hasPressedDownFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_DOWN_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_DOWN_PRISM];
}

int hasPressedLFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_L_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_L_PRISM];
}

int hasPressedRFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_R_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_R_PRISM];
}

int hasPressedStartFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_START_PRISM] && gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_START_PRISM];
}

int hasPressedAbortFlankSingle(int i) {
	return !gPrismGeneralInputData.mStatus[i].mAbortPrev && gPrismGeneralInputData.mStatus[i].mAbortCurrent;
}

int hasShotGunFlankSingle(int i)
{
	return !gPrismGeneralInputData.mStatus[i].mShotPrev && gPrismGeneralInputData.mStatus[i].mShotCurrent;
}


int hasPressedAFlank() {
	return hasPressedAFlankSingle(getMainController());
}

int hasPressedBFlank() {
	return hasPressedBFlankSingle(getMainController());
}

int hasPressedXFlank() {
	return hasPressedXFlankSingle(getMainController());
}

int hasPressedYFlank() {
	return hasPressedYFlankSingle(getMainController());
}

int hasPressedLeftFlank() {
	return hasPressedLeftFlankSingle(getMainController());
}

int hasPressedRightFlank() {
	return hasPressedRightFlankSingle(getMainController());
}

int hasPressedUpFlank() {
	return hasPressedUpFlankSingle(getMainController());
}

int hasPressedDownFlank() {
	return hasPressedDownFlankSingle(getMainController());
}

int hasPressedLFlank() {
	return hasPressedLFlankSingle(getMainController());
}

int hasPressedRFlank() {
	return hasPressedRFlankSingle(getMainController());
}

int hasPressedStartFlank() {
	return hasPressedStartFlankSingle(getMainController());
}

int hasPressedAbortFlank() {
	return hasPressedAbortFlankSingle(getMainController());
}



int hasPressedA() {
	return hasPressedASingle(getMainController());
}

int hasPressedB() {
	return hasPressedBSingle(getMainController());
}

int hasPressedX() {
	return hasPressedXSingle(getMainController());
}

int hasPressedY() {
	return hasPressedYSingle(getMainController());
}

int hasPressedLeft() {
	return hasPressedLeftSingle(getMainController());
}

int hasPressedRight() {
	return hasPressedRightSingle(getMainController());
}

int hasPressedUp() {
	return hasPressedUpSingle(getMainController());
}

int hasPressedDown() {
	return hasPressedDownSingle(getMainController());
}

int hasPressedL() {
	return hasPressedLSingle(getMainController());
}

int hasPressedR() {
	return hasPressedRSingle(getMainController());
}

int hasPressedStart() {
	return hasPressedStartSingle(getMainController());
}

int hasPressedAbort() {
	return hasPressedAbortSingle(getMainController());
}

double getLeftStickNormalizedX() {
	return getSingleLeftStickNormalizedX(getMainController());
}

double getLeftStickNormalizedY() {
	return getSingleLeftStickNormalizedY(getMainController());
}

double getLNormalized() {
	return getSingleLNormalized(getMainController());
}

double getRNormalized() {
	return getSingleRNormalized(getMainController());
}

int hasPressedAnyButton() {
	int hasPressedFaceButton = hasPressedA() || hasPressedB() || hasPressedX() || hasPressedY() || hasPressedStart();
	int hasPressedShoulderButton = hasPressedR() || hasPressedL();
	int hasPressedDirection = hasPressedLeft() || hasPressedRight() || hasPressedUp() || hasPressedDown();

	return hasPressedFaceButton || hasPressedShoulderButton || hasPressedDirection;
}

int hasShotGun()
{
	return hasShotGunSingle(getMainController());
}

int hasShotGunFlank()
{
	return hasShotGunFlankSingle(getMainController());
}

Vector3D getShotPosition()
{
	return getShotPositionSingle(getMainController());
}


void setMainController(int i) {
	gPrismGeneralInputData.mMainController = i;
}

int getMainController() {
	return gPrismGeneralInputData.mMainController;
}

int isUsingController() {
	return isUsingControllerSingle(getMainController());
}

double getFishingRodIntensity() {
	return getFishingRodIntensitySingle(getMainController());
}

double getFishingRodIntensitySingle(int i) {
	return getSingleRNormalized(i);
}

void addControllerRumbleBasic(Duration tDuration)
{
	addControllerRumbleBasicSingle(getMainController(), tDuration);
}

void addControllerRumble(Duration tDuration, int tFrequency, double tAmplitude) {
addControllerRumbleSingle(getMainController(), tDuration, tFrequency, tAmplitude);
}

void turnControllerRumbleOn(int tFrequency, double tAmplitude) {
	turnControllerRumbleOnSingle(getMainController(), tFrequency, tAmplitude);
}

void turnControllerRumbleOff() {
	turnControllerRumbleOffSingle(getMainController());
}

void addControllerRumbleBasicSingle(int i, Duration tDuration)
{
	addControllerRumbleSingle(i, tDuration, 10, 1);
}

void turnControllerRumbleOnSingle(int i, int tFrequency, double tAmplitude) {
	addControllerRumbleSingle(i, INF, tFrequency, tAmplitude);
}

static void updateInputSettingController(int i) {
	int pressedAnyButton = 0;
	for (int j = 0; j < CONTROLLER_BUTTON_AMOUNT_PRISM; j++) {
		if (hasPressedRawButton(i, (ControllerButtonPrism)j)) {
			pressedAnyButton = 1;
			if (!gPrismGeneralInputData.mSetInput[i].mFlankCheckDone) continue;
			setButtonForController(i, gPrismGeneralInputData.mSetInput[i].mTargetButton, (ControllerButtonPrism)j);
			gPrismGeneralInputData.mSetInput[i].mIsActive = 0;
			if (gPrismGeneralInputData.mSetInput[i].mSettingOptionalCB) gPrismGeneralInputData.mSetInput[i].mSettingOptionalCB(gPrismGeneralInputData.mSetInput[i].mCaller);
			break;
		}
	}

	if (!pressedAnyButton) gPrismGeneralInputData.mSetInput[i].mFlankCheckDone = 1;
}

static void updateInputSettingKeyboard(int i) {
	int pressedAnyKey = 0;
	for (int j = 0; j < KEYBOARD_AMOUNT_PRISM; j++) {
		if (hasPressedRawKeyboardKey((KeyboardKeyPrism)j)) {
			pressedAnyKey = 1;
			if (!gPrismGeneralInputData.mSetInput[i].mFlankCheckDone) continue;
			setButtonForKeyboard(i, gPrismGeneralInputData.mSetInput[i].mTargetButton, (KeyboardKeyPrism)j);
			gPrismGeneralInputData.mSetInput[i].mIsActive = 0;
			if(gPrismGeneralInputData.mSetInput[i].mSettingOptionalCB) gPrismGeneralInputData.mSetInput[i].mSettingOptionalCB(gPrismGeneralInputData.mSetInput[i].mCaller);
			break;
		}
	}

	if (!pressedAnyKey) gPrismGeneralInputData.mSetInput[i].mFlankCheckDone = 1;
}

static void updateInputGettingController(int i) {
	int pressedAnyButton = 0;
	for (int j = 0; j < CONTROLLER_BUTTON_AMOUNT_PRISM; j++) {
		if (hasPressedRawButton(i, (ControllerButtonPrism)j)) {
			pressedAnyButton = 1;
			if (!gPrismGeneralInputData.mSetInput[i].mFlankCheckDone) continue;
			gPrismGeneralInputData.mSetInput[i].mIsActive = 0;
			gPrismGeneralInputData.mSetInput[i].mControllerWaitingCB(gPrismGeneralInputData.mSetInput[i].mCaller, (ControllerButtonPrism)j);
			break;
		}
	}

	if (!pressedAnyButton) gPrismGeneralInputData.mSetInput[i].mFlankCheckDone = 1;
}

static void updateInputGettingKeyboard(int i) {
	for (int j = 0; j < KEYBOARD_AMOUNT_PRISM; j++) {
		if (hasPressedKeyboardKeyFlank((KeyboardKeyPrism)j)) {
			gPrismGeneralInputData.mSetInput[i].mIsActive = 0;
			gPrismGeneralInputData.mSetInput[i].mKeyboardWaitingCB(gPrismGeneralInputData.mSetInput[i].mCaller, (KeyboardKeyPrism)j);
			break;
		}
	}
}

static void updateInputSettingSingle(int i) {
	if (!gPrismGeneralInputData.mSetInput[i].mIsActive) return;

	if (gPrismGeneralInputData.mSetInput[i].mIsSetting) {
		if (gPrismGeneralInputData.mSetInput[i].mIsSettingController) {
			updateInputSettingController(i);
		}
		else {
			updateInputSettingKeyboard(i);
		}
	}
	else {
		if (gPrismGeneralInputData.mSetInput[i].mIsSettingController) {
			updateInputGettingController(i);
		}
		else {
			updateInputGettingKeyboard(i);
		}
	}
}

static void updateInputSetting() {
		updateInputSettingSingle(0);
		updateInputSettingSingle(1);
}

static void updateInputFlagSingle(uint8_t* tPrev, uint8_t* tCurrent, int tUpdateValue) {
	*tPrev = *tCurrent;
	*tCurrent = (uint8_t)tUpdateValue;
}

static void updateInputFlanks() {
	for (int i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_A_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_A_PRISM], hasPressedASingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_B_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_B_PRISM], hasPressedBSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_X_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_X_PRISM], hasPressedXSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_Y_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_Y_PRISM], hasPressedYSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_LEFT_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_LEFT_PRISM], hasPressedLeftSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_RIGHT_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_RIGHT_PRISM], hasPressedRightSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_UP_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_UP_PRISM], hasPressedUpSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_DOWN_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_DOWN_PRISM], hasPressedDownSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_L_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_L_PRISM], hasPressedLSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_R_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_R_PRISM], hasPressedRSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mPrev[CONTROLLER_START_PRISM], &gPrismGeneralInputData.mStatus[i].mCurrent[CONTROLLER_START_PRISM], hasPressedStartSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mAbortPrev, &gPrismGeneralInputData.mStatus[i].mAbortCurrent, hasPressedAbortSingle(i));
		updateInputFlagSingle(&gPrismGeneralInputData.mStatus[i].mShotPrev, &gPrismGeneralInputData.mStatus[i].mShotCurrent, hasShotGunSingle(i));
	}
}

extern void updateInputPlatform();

void updateInput() {
	updateInputPlatform();
	updateInputSetting();
	updateInputFlanks();
}

static void setButtonFromUserInputGeneral(int i, ControllerButtonPrism tTargetButton, void(*tSettingOptionalCB)(void*), void(*tControllerWaitingCB)(void*, ControllerButtonPrism), void(*tKeyboardWaitingCB)(void*, KeyboardKeyPrism), void* tCaller, int tIsSetting, int tIsSettingController) {
	gPrismGeneralInputData.mSetInput[i].mTargetButton = tTargetButton;
	gPrismGeneralInputData.mSetInput[i].mIsSetting = tIsSetting;
	gPrismGeneralInputData.mSetInput[i].mIsSettingController = tIsSettingController;
	gPrismGeneralInputData.mSetInput[i].mSettingOptionalCB = tSettingOptionalCB;
	gPrismGeneralInputData.mSetInput[i].mControllerWaitingCB = tControllerWaitingCB;
	gPrismGeneralInputData.mSetInput[i].mKeyboardWaitingCB = tKeyboardWaitingCB;
	gPrismGeneralInputData.mSetInput[i].mCaller = tCaller;
	gPrismGeneralInputData.mSetInput[i].mFlankCheckDone = 0;
	gPrismGeneralInputData.mSetInput[i].mIsActive = 1;
}

void setButtonFromUserInputForController(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*), void* tCaller) {
	setButtonFromUserInputGeneral(i, tTargetButton, tOptionalCB, NULL, NULL, tCaller, 1, 1);
}

void setButtonFromUserInputForKeyboard(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*), void* tCaller) {
	setButtonFromUserInputGeneral(i, tTargetButton, tOptionalCB, NULL, NULL, tCaller, 1, 0);
}

void waitForButtonFromUserInputForController(int i, void(*tCB)(void *, ControllerButtonPrism), void * tCaller)
{
	setButtonFromUserInputGeneral(i, CONTROLLER_A_PRISM, NULL, tCB, NULL, tCaller, 0, 0);
}

void waitForButtonFromUserInputForKeyboard(int i, void(*tCB)(void*, KeyboardKeyPrism), void* tCaller) {
	setButtonFromUserInputGeneral(i, CONTROLLER_A_PRISM, NULL, NULL, tCB, tCaller, 0, 0);
}

void cancelWaitingForButtonFromUserInput(int i)
{
	gPrismGeneralInputData.mSetInput[i].mIsActive = 0;
}
