#include "prism/input.h"

#include "prism/log.h"
#include "prism/math.h"

typedef struct {
	int mAFlank;
	int mBFlank;
	int mXFlank;
	int mYFlank;
	int mLFlank;
	int mRFlank;
	int mLeftFlank;
	int mRightFlank;
	int mUpFlank;
	int mDownFlank;
	int mStartFlank;
	int mAbortFlank;
	int mShotFlank;
} InputFlanks;

typedef struct {
	int mIsActive;
	ControllerButtonPrism mTargetButton;
	int mIsSettingController;
	int mFlankCheckDone;
	void(*mOptionalCB)(void*); 
	void* mCaller;
} SetInputData;

static struct {
	InputFlanks mFlanks[MAXIMUM_CONTROLLER_AMOUNT];
	SetInputData mSetInput[MAXIMUM_CONTROLLER_AMOUNT];

	int mMainController;
} gData;

void resetInputForAllControllers() {
	int i;
	for (i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		hasPressedAFlankSingle(i);
		hasPressedBFlankSingle(i);
		hasPressedXFlankSingle(i);
		hasPressedYFlankSingle(i);
		hasPressedLFlankSingle(i);
		hasPressedRFlankSingle(i);
		hasPressedLeftFlankSingle(i);
		hasPressedRightFlankSingle(i);
		hasPressedUpFlankSingle(i);
		hasPressedDownFlankSingle(i);
		hasPressedStartFlankSingle(i);
		hasPressedAbortFlankSingle(i);
		hasShotGunFlankSingle(i);

		gData.mSetInput[i].mIsActive = 0;
	}
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


int hasPressedAFlankSingle(int i) {
	return hasPressedFlank(hasPressedASingle(i), &gData.mFlanks[i].mAFlank);
}

int hasPressedBFlankSingle(int i) {
	return hasPressedFlank(hasPressedBSingle(i), &gData.mFlanks[i].mBFlank);
}
int hasPressedXFlankSingle(int i) {
	return hasPressedFlank(hasPressedXSingle(i), &gData.mFlanks[i].mXFlank);
}

int hasPressedYFlankSingle(int i) {
	return hasPressedFlank(hasPressedYSingle(i), &gData.mFlanks[i].mYFlank);
}

int hasPressedLeftFlankSingle(int i) {
	return hasPressedFlank(hasPressedLeftSingle(i), &gData.mFlanks[i].mLeftFlank);
}

int hasPressedRightFlankSingle(int i) {
	return hasPressedFlank(hasPressedRightSingle(i), &gData.mFlanks[i].mRightFlank);
}

int hasPressedUpFlankSingle(int i) {
	return hasPressedFlank(hasPressedUpSingle(i), &gData.mFlanks[i].mUpFlank);
}

int hasPressedDownFlankSingle(int i) {
	return hasPressedFlank(hasPressedDownSingle(i), &gData.mFlanks[i].mDownFlank);
}

int hasPressedLFlankSingle(int i) {
	return hasPressedFlank(hasPressedLSingle(i), &gData.mFlanks[i].mLFlank);
}

int hasPressedRFlankSingle(int i) {
	return hasPressedFlank(hasPressedRSingle(i), &gData.mFlanks[i].mRFlank);
}

int hasPressedStartFlankSingle(int i) {
	return hasPressedFlank(hasPressedStartSingle(i), &gData.mFlanks[i].mStartFlank);
}

int hasPressedAbortFlankSingle(int i) {
	return hasPressedFlank(hasPressedAbortSingle(i), &gData.mFlanks[i].mAbortFlank);
}

int hasShotGunFlankSingle(int i)
{
	return hasPressedFlank(hasShotGunSingle(i), &gData.mFlanks[i].mShotFlank);
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

int hasPressedAnyButton() { // TODO: smarter
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
	gData.mMainController = i;
}

int getMainController() {
	return gData.mMainController;
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
			if (!gData.mSetInput[i].mFlankCheckDone) continue;
			setButtonForController(i, gData.mSetInput[i].mTargetButton, (ControllerButtonPrism)j);
			gData.mSetInput[i].mIsActive = 0;
			if (gData.mSetInput[i].mOptionalCB) gData.mSetInput[i].mOptionalCB(gData.mSetInput[i].mCaller);
			break;
		}
	}

	if (!pressedAnyButton) gData.mSetInput[i].mFlankCheckDone = 1;
}

static void updateInputSettingKeyboard(int i) {
	int pressedAnyKey = 0;
	for (int j = 0; j < KEYBOARD_AMOUNT_PRISM; j++) {
		if (hasPressedRawKeyboardKey((KeyboardKeyPrism)j)) {
			pressedAnyKey = 1;
			if (!gData.mSetInput[i].mFlankCheckDone) continue;
			setButtonForKeyboard(i, gData.mSetInput[i].mTargetButton, (KeyboardKeyPrism)j);
			gData.mSetInput[i].mIsActive = 0;
			if(gData.mSetInput[i].mOptionalCB) gData.mSetInput[i].mOptionalCB(gData.mSetInput[i].mCaller);
			break;
		}
	}

	if (!pressedAnyKey) gData.mSetInput[i].mFlankCheckDone = 1;
}

static void updateInputSettingSingle(int i) {
	if (!gData.mSetInput[i].mIsActive) return;

	if (gData.mSetInput[i].mIsSettingController) {
		updateInputSettingController(i);
	}
	else {
		updateInputSettingKeyboard(i);
	}
}

static void updateInputSetting() {
		updateInputSettingSingle(0);
		updateInputSettingSingle(1);
}

extern void updateInputPlatform();

void updateInput() {
	updateInputPlatform();
	updateInputSetting();
}

static void setButtonFromUserInputGeneral(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*), void* tCaller, int tIsSettingController) {
	gData.mSetInput[i].mTargetButton = tTargetButton;
	gData.mSetInput[i].mIsSettingController = tIsSettingController;
	gData.mSetInput[i].mOptionalCB = tOptionalCB;
	gData.mSetInput[i].mCaller = tCaller;
	gData.mSetInput[i].mFlankCheckDone = 0;
	gData.mSetInput[i].mIsActive = 1;
}

void setButtonFromUserInputForController(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*), void* tCaller) {
	setButtonFromUserInputGeneral(i, tTargetButton, tOptionalCB, tCaller, 1);
}

void setButtonFromUserInputForKeyboard(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*), void* tCaller) {
	setButtonFromUserInputGeneral(i, tTargetButton, tOptionalCB, tCaller, 0);
}