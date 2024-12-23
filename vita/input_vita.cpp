#include "prism/input.h"

#include <stdint.h>
#include <stdarg.h>
#include <queue>

#include <psp2/ctrl.h> 

#include "prism/log.h"
#include "prism/math.h"
#include "prism/clipboardhandler.h"

using namespace std;
namespace prism {

	typedef struct {
		SceCtrlData mState;
	} Controller;

	static struct {
		Controller mController;
	} gPrismVitaInputData;
	
	static void updateController();

	static void initControllerState()
	{
		memset(&gPrismVitaInputData.mController.mState, 0, sizeof(SceCtrlData));
		updateController();
	}


	static int evaluateVitaButtonA(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_CROSS;
	}

	static int evaluateVitaButtonB(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_CIRCLE;
	}

	static int evaluateVitaButtonX(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_SQUARE;
	}

	static int evaluateVitaButtonY(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_TRIANGLE;
	}

	static int evaluateVitaButtonL(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_L1;
	}

	static int evaluateVitaButtonR(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_R1;
	}

	static int evaluateVitaButtonLeft(int i) {
		if (i) return 0;
		int state = 0;
		state |= gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_LEFT;
		state |= (getSingleLeftStickNormalizedX(i) < -0.5);
		return state;
	}

	static int evaluateVitaButtonRight(int i) {
		if (i) return 0;
		int state = 0;
		state |= gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_RIGHT;
		state |= (getSingleLeftStickNormalizedX(i) > 0.5);
		return state;
	}

	static int evaluateVitaButtonUp(int i) {
		if (i) return 0;
		int state = 0;
		state |= gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_UP;
		state |= (getSingleLeftStickNormalizedY(i) < -0.5);
		return state;
	}

	static int evaluateVitaButtonDown(int i) {
		if (i) return 0;
		int state = 0;
		state |= gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_DOWN;
		state |= (getSingleLeftStickNormalizedY(i) > 0.5);
		return state;
	}

	static int evaluateVitaButtonStart(int i) {
		if (i) return 0;
		return gPrismVitaInputData.mController.mState.buttons & SCE_CTRL_START;
	}

	typedef int(*InputEvaluationFunction)(int);

	static InputEvaluationFunction gVitaButtonMapping[] = {
		evaluateVitaButtonA,
		evaluateVitaButtonB,
		evaluateVitaButtonX,
		evaluateVitaButtonY,
		evaluateVitaButtonL,
		evaluateVitaButtonR,
		evaluateVitaButtonLeft,
		evaluateVitaButtonRight,
		evaluateVitaButtonUp,
		evaluateVitaButtonDown,
		evaluateVitaButtonStart,
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

	void initInput() {
		sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
		initControllerState();
	}

	static void updateController() {
		sceCtrlPeekBufferPositive2(0, &gPrismVitaInputData.mController.mState, 1);
	}

	void updateInputPlatform() {
		updateController();
	}

	int hasPressedASingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_A_PRISM]](i);
	}

	int hasPressedBSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_B_PRISM]](i);
	}

	int hasPressedXSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_X_PRISM]](i);
	}

	int hasPressedYSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_Y_PRISM]](i);
	}

	int hasPressedLeftSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_LEFT_PRISM]](i);
	}

	int hasPressedRightSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_RIGHT_PRISM]](i);
	}

	int hasPressedUpSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_UP_PRISM]](i);
	}

	int hasPressedDownSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_DOWN_PRISM]](i);
	}

	int hasPressedLSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_L_PRISM]](i);
	}

	int hasPressedRSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_R_PRISM]](i);
	}

	int hasPressedStartSingle(int i) {
		return gVitaButtonMapping[gButtonMapping[i][CONTROLLER_START_PRISM]](i);
	}

	int hasPressedAbortSingle(int i) {
		return hasPressedASingle(i) && hasPressedBSingle(i) && hasPressedXSingle(i) && hasPressedYSingle(i) && hasPressedStartSingle(i);
	}

	int hasShotGunSingle(int /*i*/)
	{
		// UNSUPPORTED
		return 0;
	}


	Vector3D getShotPositionSingle(int /*i*/) {
		// UNSUPPORTED
		return Vector3D(0, 0, 0);
	}


	double getSingleLeftStickNormalizedX(int /*i*/) {
		return (((int)gPrismVitaInputData.mController.mState.lx) - 127) / 128.0;
	}

	double getSingleLeftStickNormalizedY(int /*i*/) {
		return (((int)gPrismVitaInputData.mController.mState.ly) - 127) / 128.0;
	}

	double getSingleLNormalized(int i) {
		return evaluateVitaButtonL(i) ? 1.0 : 0.0;
	}

	double getSingleRNormalized(int i) {
		return evaluateVitaButtonR(i) ? 1.0 : 0.0;
	}

	void forceMouseCursorToWindow() {
		// UNSUPPORTED
	}

	void releaseMouseCursorFromWindow() {
		// UNSUPPORTED
	}

	int isUsingControllerSingle(int i) {
		return !i;
	}

	void addControllerRumbleSingle(int i, Duration tDuration, int tFrequency, double tAmplitude) {
		// UNSUPPORTED
	}

	void turnControllerRumbleOffSingle(int i) {
		// UNSUPPORTED
	}

	int hasPressedRawButton(int i, ControllerButtonPrism tButton) {
		return gVitaButtonMapping[tButton](i);
	}

	int hasPressedRawKeyboardKey(KeyboardKeyPrism tKey) {
		// UNSUPPORTED
		return 0;
	}

	int hasPressedKeyboardKeyFlank(KeyboardKeyPrism tKey) {
		// UNSUPPORTED
		return 0;
	}

	int hasPressedKeyboardMultipleKeyFlank(int tKeyAmount, ...) {
		if (!tKeyAmount) return 0;
		// UNSUPPORTED
		return 0;
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
		// UNSUPPORTED
		return (KeyboardKeyPrism)0;
	}

	void setButtonForKeyboard(int i, ControllerButtonPrism tTargetButton, KeyboardKeyPrism tKeyValue)
	{
		// UNSUPPORTED
	}

	void waitForCharacterFromUserInput(int /*i*/, void(*tCB)(void*, const std::string&), void* tCaller) {
		// UNSUPPORTED
	}

	void cancelWaitingForCharacterFromUserInput(int /*i*/) {
		// UNSUPPORTED
	}

	int getInputDelay()
	{
		return 0;
	}

	void setInputDelay(int tInputDelay) {
		// UNSUPPORTED
	}

	void setInputBufferSize(int tInputBufferSize)
	{
		// UNSUPPORTED
	}

	Vector2D getMousePointerPosition()
	{
		// UNSUPPORTED
		return Vector2D(0, 0);
	}

	bool isMouseInRectangle(const GeoRectangle2D& tRectangle)
	{
		const auto pos = getMousePointerPosition();
		return pos.x >= tRectangle.mTopLeft.x && pos.x <= tRectangle.mBottomRight.x && pos.y >= tRectangle.mTopLeft.y && pos.y <= tRectangle.mBottomRight.y;

	}
	bool hasPressedMouseLeft()
	{
		return hasPressedMouseLeftSingle(0);
	}

	bool hasPressedMouseLeftSingle(int)
	{
		// TODO
		return 0;
	}

	bool hasPressedMouseRight()
	{
		return hasPressedMouseRightSingle(0);
	}

	bool hasPressedMouseRightSingle(int)
	{
		// TODO
		return 0;
	}
}