#include "prism/input.h"

#include <stdint.h>
#include <stdarg.h>

#include <SDL.h>

#include "prism/log.h"
#include "prism/math.h"
#include "prism/clipboardhandler.h"

using namespace std;

typedef struct {
	int mIsUsingController;
	SDL_GameController* mController;
	SDL_Haptic* mHaptic;

	int mIsRumbling;
	SDL_HapticEffect mHapticEffect;
	int mHapticEffectID;
	Duration mRumbleNow;
	Duration mRumbleDuration;
} Controller;

typedef struct {
	int mIsActive;
	void(*mCB)(void*, const std::string&);
	void* mCaller;
} KeyInputWait;

static struct {
	KeyInputWait mInputWait;

	const Uint8* mKeyStatePointer;
	Uint8* mCurrentKeyStates;
	Uint8* mPreviousKeyStates;
	Controller mControllers[MAXIMUM_CONTROLLER_AMOUNT];
} gPrismWindowsInputData;



static int evaluateSDLButtonA(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_A);
}

static int evaluateSDLButtonB(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_B);
}

static int evaluateSDLButtonX(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_X);
}

static int evaluateSDLButtonY(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_Y);
}

static int evaluateSDLButtonL(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;	
	double axis = SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0;
	return (axis > 0.5);
}

static int evaluateSDLButtonR(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	double axis = SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0;
	return (axis > 0.5);
}

static int evaluateSDLButtonLeft(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	double axis = SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0;
	int ret = (axis < -0.5);
	ret |= SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	return ret;
}

static int evaluateSDLButtonRight(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	double axis = SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0;
	int ret = (axis > 0.5);
	ret |= SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	return ret;
}

static int evaluateSDLButtonUp(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	double axis = SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0;
	int ret = (axis < -0.5);
	ret |= SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_DPAD_UP);
	return ret;
}

static int evaluateSDLButtonDown(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	double axis = SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0;
	int ret = (axis > 0.5);
	ret |= SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	return ret;
}

static int evaluateSDLButtonStart(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_START);
}

typedef int(*InputEvaluationFunction)(int);

static InputEvaluationFunction gSDLButtonMapping[] = {
	evaluateSDLButtonA,
	evaluateSDLButtonB,
	evaluateSDLButtonX,
	evaluateSDLButtonY,
	evaluateSDLButtonL,
	evaluateSDLButtonR,
	evaluateSDLButtonLeft,
	evaluateSDLButtonRight,
	evaluateSDLButtonUp,
	evaluateSDLButtonDown,
	evaluateSDLButtonStart,
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



static std::pair<SDL_Keycode, SDL_Scancode> gPrismToSDLKeyboardMapping[] = {
	make_pair(SDLK_a, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_b, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_c, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_d, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_e, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_f, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_g, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_h, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_i, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_j, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_k, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_l, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_m , SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_n, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_o, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_p, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_q, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_r, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_s, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_t, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_u, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_v, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_w, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_x, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_y, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_z, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_0, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_1, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_2, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_3, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_4, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_5, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_6, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_7, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_8, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_9, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_SPACE, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_LEFT, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_RIGHT, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_UP, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_DOWN, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F1, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F2, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F3, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F4, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F5, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F6, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F7, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F8, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F9, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F10, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F11, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_F12, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_SCROLLLOCK, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_PAUSE, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_CARET, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_LCTRL, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_LSHIFT, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_RETURN, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_BACKSPACE, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_DELETE, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_PERIOD, SDL_SCANCODE_UNKNOWN),
	make_pair(SDLK_SLASH, SDL_SCANCODE_SLASH),
};

static KeyboardKeyPrism gKeys[MAXIMUM_CONTROLLER_AMOUNT][KEYBOARD_AMOUNT_PRISM] = {
	{
		KEYBOARD_A_PRISM,
		KEYBOARD_S_PRISM,
		KEYBOARD_Q_PRISM,
		KEYBOARD_W_PRISM,
		KEYBOARD_E_PRISM,
		KEYBOARD_D_PRISM,
		KEYBOARD_LEFT_PRISM,
		KEYBOARD_RIGHT_PRISM,
		KEYBOARD_UP_PRISM,
		KEYBOARD_DOWN_PRISM,
		KEYBOARD_RETURN_PRISM,
	},
	{
		KEYBOARD_H_PRISM,
		KEYBOARD_J_PRISM,
		KEYBOARD_Y_PRISM,
		KEYBOARD_U_PRISM,
		KEYBOARD_I_PRISM,
		KEYBOARD_K_PRISM,
		KEYBOARD_4_PRISM,
		KEYBOARD_6_PRISM,
		KEYBOARD_8_PRISM,
		KEYBOARD_2_PRISM,
		KEYBOARD_3_PRISM,
	},
};

static void initKeyboardScancodes() {
	for (int i = 0; i < KEYBOARD_AMOUNT_PRISM; i++) {
		auto scancode = SDL_GetScancodeFromKey(gPrismToSDLKeyboardMapping[i].first);
		if (scancode != SDL_SCANCODE_UNKNOWN) gPrismToSDLKeyboardMapping[i].second = scancode;
	}
}

void initInput() {
	gPrismWindowsInputData.mInputWait.mIsActive = 0;

	gPrismWindowsInputData.mPreviousKeyStates = (Uint8*)allocClearedMemory(SDL_NUM_SCANCODES, 1);
	gPrismWindowsInputData.mCurrentKeyStates = (Uint8*)allocClearedMemory(SDL_NUM_SCANCODES, 1);
	gPrismWindowsInputData.mKeyStatePointer = SDL_GetKeyboardState(NULL);
	initKeyboardScancodes();
}

static void loadController(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) return;

	gPrismWindowsInputData.mControllers[i].mController = SDL_GameControllerOpen(i);
	gPrismWindowsInputData.mControllers[i].mHaptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gPrismWindowsInputData.mControllers[i].mController));
	gPrismWindowsInputData.mControllers[i].mIsRumbling = 0;
	gPrismWindowsInputData.mControllers[i].mIsUsingController = 1;
}

static void unloadController(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return;
	turnControllerRumbleOffSingle(i);
	if(gPrismWindowsInputData.mControllers[i].mHaptic) SDL_HapticClose(gPrismWindowsInputData.mControllers[i].mHaptic);
	SDL_GameControllerClose(gPrismWindowsInputData.mControllers[i].mController);
	gPrismWindowsInputData.mControllers[i].mController = NULL;
	gPrismWindowsInputData.mControllers[i].mIsUsingController = 0;
}

static void updateSingleControllerInput(int i) {
	if (i >= SDL_NumJoysticks()) {
		unloadController(i);
		return;
	}

	loadController(i);
}

static void updateSingleControllerRumble(int i) {
	if (!isUsingControllerSingle(i)) return;
	if (!gPrismWindowsInputData.mControllers[i].mIsRumbling) return;

	if (handleDurationAndCheckIfOver(&gPrismWindowsInputData.mControllers[i].mRumbleNow, gPrismWindowsInputData.mControllers[i].mRumbleDuration)) {
		turnControllerRumbleOffSingle(i);
	}
}

static void updateControllers() {
	int i;
	for (i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		updateSingleControllerInput(i);
		updateSingleControllerRumble(i);
	}
}

void updateInputPlatform() {
	memcpy(gPrismWindowsInputData.mPreviousKeyStates, gPrismWindowsInputData.mCurrentKeyStates, SDL_NUM_SCANCODES);
	memcpy(gPrismWindowsInputData.mCurrentKeyStates, gPrismWindowsInputData.mKeyStatePointer, SDL_NUM_SCANCODES);
	updateControllers();
}

int hasPressedASingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_A_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_A_PRISM]](i);
	return state;
}

int hasPressedBSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_B_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_B_PRISM]](i);
	return state;
}

int hasPressedXSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_X_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_X_PRISM]](i);
	return state;
}

int hasPressedYSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_Y_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_Y_PRISM]](i);
	return state;
}

int hasPressedLeftSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_LEFT_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_LEFT_PRISM]](i);
	return state;
}

int hasPressedRightSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_RIGHT_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_RIGHT_PRISM]](i);
	return state;
}

int hasPressedUpSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_UP_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_UP_PRISM]](i);
	return state;
}

int hasPressedDownSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_DOWN_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_DOWN_PRISM]](i);
	return state;
}

int hasPressedLSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_L_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_L_PRISM]](i);
	return state;
}

int hasPressedRSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_R_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_R_PRISM]](i);
	return state;
}

int hasPressedStartSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[gPrismToSDLKeyboardMapping[gKeys[i][CONTROLLER_START_PRISM]].second];
	state |= gSDLButtonMapping[gButtonMapping[i][CONTROLLER_START_PRISM]](i);
	return state;
}

int hasPressedAbortSingle(int i) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;
	int state = gPrismWindowsInputData.mCurrentKeyStates[SDL_SCANCODE_ESCAPE];
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		state |= (hasPressedASingle(i) && hasPressedBSingle(i) && hasPressedXSingle(i) && hasPressedYSingle(i) && hasPressedStartSingle(i));
	}
	return state;
}

int hasShotGunSingle(int /*i*/)
{
	uint32_t mask = SDL_GetMouseState(NULL, NULL);
	return mask & SDL_BUTTON(SDL_BUTTON_LEFT);
}

extern Vector3D correctSDLWindowPosition(Vector3D v);

Vector3D getShotPositionSingle(int /*i*/) {
	int x, y;
	SDL_GetMouseState(&x, &y);
	Vector3D ret = makePosition(x, y, 0);
	return correctSDLWindowPosition(ret);
}


static double getStickNormalizedBinary(int tCodeMinus, int tCodePlus) {
	if (gPrismWindowsInputData.mCurrentKeyStates == NULL) return 0;

	if (gPrismWindowsInputData.mCurrentKeyStates[tCodeMinus]) return -1;
	else if (gPrismWindowsInputData.mCurrentKeyStates[tCodePlus]) return 1;
	else return 0;
}

double getSingleLeftStickNormalizedX(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0;
	}
	else return getStickNormalizedBinary(gKeys[i][CONTROLLER_LEFT_PRISM], gKeys[i][CONTROLLER_RIGHT_PRISM]);
}

double getSingleLeftStickNormalizedY(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0;
	}
	else return getStickNormalizedBinary(gKeys[i][CONTROLLER_UP_PRISM], gKeys[i][CONTROLLER_DOWN_PRISM]);
}

double getSingleLNormalized(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0;
	}
	else return gPrismWindowsInputData.mCurrentKeyStates[gKeys[i][CONTROLLER_L_PRISM]];
}

double getSingleRNormalized(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0;
	}
	else return gPrismWindowsInputData.mCurrentKeyStates[gKeys[i][CONTROLLER_R_PRISM]];
}

extern SDL_Window* gSDLWindow;

void forceMouseCursorToWindow() {
#ifdef __EMSCRIPTEN__
	return;
#endif
	SDL_SetWindowGrab(gSDLWindow, SDL_TRUE);
}

void releaseMouseCursorFromWindow() {
#ifdef __EMSCRIPTEN__
	return;
#endif
	SDL_SetWindowGrab(gSDLWindow, SDL_FALSE);
}

int isUsingControllerSingle(int i) {
	return gPrismWindowsInputData.mControllers[i].mIsUsingController;
}

void addControllerRumbleSingle(int i, Duration tDuration, int tFrequency, double tAmplitude) {
	if (!isUsingControllerSingle(i)) return;
	if (!gPrismWindowsInputData.mControllers[i].mHaptic) return;

	turnControllerRumbleOffSingle(i);

	SDL_HapticEffect* effect = &gPrismWindowsInputData.mControllers[i].mHapticEffect;
	memset(effect, 0, sizeof(SDL_HapticEffect));

	if ((SDL_HapticQuery(gPrismWindowsInputData.mControllers[i].mHaptic) & SDL_HAPTIC_SINE)) {
		effect->type = SDL_HAPTIC_SINE;
	}
	else {
		effect->type = SDL_HAPTIC_LEFTRIGHT;
	}	
	effect->periodic.direction.type = SDL_HAPTIC_POLAR; 
	effect->periodic.direction.dir[0] = 18000; 
	effect->periodic.period = (uint16_t)tFrequency; 
	
	effect->periodic.magnitude = (int16_t)(INT16_MAX * fclamp(tAmplitude, 0, 1)); 
	effect->periodic.length = 5000;
	effect->periodic.attack_length = 1000;
	effect->periodic.fade_length = 1000;

	gPrismWindowsInputData.mControllers[i].mHapticEffectID = SDL_HapticNewEffect(gPrismWindowsInputData.mControllers[i].mHaptic, effect);

	SDL_HapticRunEffect(gPrismWindowsInputData.mControllers[i].mHaptic, gPrismWindowsInputData.mControllers[i].mHapticEffectID, 1);

	gPrismWindowsInputData.mControllers[i].mRumbleNow = 0;
	gPrismWindowsInputData.mControllers[i].mRumbleDuration = tDuration;
	gPrismWindowsInputData.mControllers[i].mIsRumbling = 1;
}

void turnControllerRumbleOffSingle(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsRumbling) return;

	SDL_HapticDestroyEffect(gPrismWindowsInputData.mControllers[i].mHaptic, gPrismWindowsInputData.mControllers[i].mHapticEffectID);
	gPrismWindowsInputData.mControllers[i].mIsRumbling = 0;
}

int hasPressedRawButton(int i, ControllerButtonPrism tButton) {
	return gSDLButtonMapping[tButton](i);
}

int hasPressedRawKeyboardKey(KeyboardKeyPrism tKey) {
	int id = gPrismToSDLKeyboardMapping[tKey].second;
	return gPrismWindowsInputData.mCurrentKeyStates[id];
}

int hasPressedKeyboardKeyFlank(KeyboardKeyPrism tKey) {
	int id = gPrismToSDLKeyboardMapping[tKey].second;
	return !gPrismWindowsInputData.mPreviousKeyStates[id] && gPrismWindowsInputData.mCurrentKeyStates[id];
}

int hasPressedKeyboardMultipleKeyFlank(int tKeyAmount, ...) {
	if (!tKeyAmount) return 0;

	int i;
	va_list vl;
	va_start(vl, tKeyAmount);

	int previousKeyPressed = 1, currentKeyPressed = 1;
	for (i = 0; i < tKeyAmount; i++)
	{
		KeyboardKeyPrism singleKey = (KeyboardKeyPrism)va_arg(vl, int);
		int id = gPrismToSDLKeyboardMapping[singleKey].second;
		previousKeyPressed = previousKeyPressed && gPrismWindowsInputData.mPreviousKeyStates[id];
		currentKeyPressed = currentKeyPressed && gPrismWindowsInputData.mCurrentKeyStates[id];
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
	return gKeys[i][tTargetButton];
}

void setButtonForKeyboard(int i, ControllerButtonPrism tTargetButton, KeyboardKeyPrism tKeyValue)
{
	gKeys[i][tTargetButton] = tKeyValue;
}

void receiveCharacterInputFromSDL(const std::string& tText) {
	if (!gPrismWindowsInputData.mInputWait.mIsActive) return;
	gPrismWindowsInputData.mInputWait.mCB(gPrismWindowsInputData.mInputWait.mCaller, tText);
}

void waitForCharacterFromUserInput(int /*i*/, void(*tCB)(void*, const std::string&), void* tCaller) {
	SDL_StartTextInput();

	gPrismWindowsInputData.mInputWait.mCB = tCB;
	gPrismWindowsInputData.mInputWait.mCaller = tCaller;
	gPrismWindowsInputData.mInputWait.mIsActive = 1;
}

void cancelWaitingForCharacterFromUserInput(int /*i*/) {
	SDL_StopTextInput();
	gPrismWindowsInputData.mInputWait.mIsActive = 0;
}
