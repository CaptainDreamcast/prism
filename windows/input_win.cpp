#include "prism/input.h"

#include <stdint.h>
#include <stdarg.h>
#include <queue>

#ifdef VITA
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

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

	std::deque<std::vector<Uint8>> mButtonStates; // queue of CONTROLLER_BUTTON_AMOUNT_PRISM
} Controller;

typedef struct {
	int mIsActive;
	void(*mCB)(void*, const std::string&);
	void* mCaller;
} KeyInputWait;

typedef struct
{
	const Uint8* mKeyStatePointer;
	std::deque<std::vector<Uint8>> mKeyStates; // queue of SDL_NUM_SCANCODES
	std::deque<Uint8> mConfirmationState;
} InputKeyboard;

static struct {
	KeyInputWait mInputWait;

	int mInputDelay;
	int mInputBuffer;
	
	int mUsedKeyboard[MAXIMUM_CONTROLLER_AMOUNT];
	int mUsedKeyboardMapping[MAXIMUM_CONTROLLER_AMOUNT];
	InputKeyboard mKeyboards[MAXIMUM_CONTROLLER_AMOUNT];
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

#ifdef VITA
static int evaluateSDLButtonL(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
}

static int evaluateSDLButtonR(int i) {
	if (!gPrismWindowsInputData.mControllers[i].mIsUsingController) return 0;
	return SDL_GameControllerGetButton(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
}
#else
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
#endif

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
	make_pair(SDLK_LALT, SDL_SCANCODE_UNKNOWN),
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

static std::vector<Uint8>& getButtonStates(int i, int tNegativeFrameDelta)
{
	return *(gPrismWindowsInputData.mControllers[i].mButtonStates.rbegin() - tNegativeFrameDelta);
}

static std::vector<Uint8>& getKeyStates(int i, int tNegativeFrameDelta)
{
	return *(gPrismWindowsInputData.mKeyboards[i].mKeyStates.rbegin() - tNegativeFrameDelta);
}

static void updateKeyStateArraySize(int i)
{
	while (gPrismWindowsInputData.mKeyboards[i].mKeyStates.size() < gPrismWindowsInputData.mInputDelay + gPrismWindowsInputData.mInputBuffer)
	{
		gPrismWindowsInputData.mKeyboards[i].mKeyStates.push_front(std::vector<Uint8>(SDL_NUM_SCANCODES, 0));
		gPrismWindowsInputData.mKeyboards[i].mConfirmationState.push_front(0);
	}

	while (gPrismWindowsInputData.mKeyboards[i].mKeyStates.size() > gPrismWindowsInputData.mInputDelay + gPrismWindowsInputData.mInputBuffer)
	{
		gPrismWindowsInputData.mKeyboards[i].mKeyStates.pop_front();
		gPrismWindowsInputData.mKeyboards[i].mConfirmationState.pop_front();
	}
}

static void updateKeyStateArraySizes()
{
	for (int i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		updateKeyStateArraySize(i);
	}
}

static void updateControllerButtonStateArraySize(int i)
{
	while (gPrismWindowsInputData.mControllers[i].mButtonStates.size() < gPrismWindowsInputData.mInputDelay + gPrismWindowsInputData.mInputBuffer)
	{
		gPrismWindowsInputData.mControllers[i].mButtonStates.push_front(std::vector<Uint8>(CONTROLLER_BUTTON_AMOUNT_PRISM, 0));
	}

	while (gPrismWindowsInputData.mControllers[i].mButtonStates.size() > gPrismWindowsInputData.mInputDelay + gPrismWindowsInputData.mInputBuffer)
	{
		gPrismWindowsInputData.mControllers[i].mButtonStates.pop_front();
	}
}

static void updateControllerButtonStateArraySizes()
{
	for (int i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		updateControllerButtonStateArraySize(i);
	}
}

void initInput() {
	gPrismWindowsInputData.mInputWait.mIsActive = 0;

	gPrismWindowsInputData.mUsedKeyboard[PRISM_KEYBOARD_LOCAL] = gPrismWindowsInputData.mUsedKeyboard[PRISM_KEYBOARD_NETPLAY] = 0;
	for (int i = 0; i < MAXIMUM_CONTROLLER_AMOUNT; i++) {
		gPrismWindowsInputData.mUsedKeyboardMapping[i] = i;
	}
	gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_LOCAL].mKeyStatePointer = SDL_GetKeyboardState(NULL);
	gPrismWindowsInputData.mInputDelay = 0;
	gPrismWindowsInputData.mInputBuffer = 2;
	updateKeyStateArraySizes();
	updateControllerButtonStateArraySizes();
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

static void fillSingleButtonState(int i, int buttonIndex, std::vector<Uint8>& data) {
	data[buttonIndex] = uint8_t(gSDLButtonMapping[gButtonMapping[i][buttonIndex]](i));
}

static void updateControllerButtonStateArrayContentFillFrontWithCurrentButtonState(int i) {
	auto& data = gPrismWindowsInputData.mControllers[i].mButtonStates.front();
	for (int buttonIndex = 0; buttonIndex < CONTROLLER_BUTTON_AMOUNT_PRISM; buttonIndex++)
	{
		fillSingleButtonState(i, buttonIndex, data);
	}
}

static void updateControllerButtonStateArrayContent(int i) {
	updateControllerButtonStateArrayContentFillFrontWithCurrentButtonState(i);
	gPrismWindowsInputData.mControllers[i].mButtonStates.push_back(gPrismWindowsInputData.mControllers[i].mButtonStates.front());
	gPrismWindowsInputData.mControllers[i].mButtonStates.pop_front();
}

static void updateControllerButtonStates(int i)
{
	updateControllerButtonStateArraySize(i);
	updateControllerButtonStateArrayContent(i);
}

static void updateSingleControllerInput(int i) {
	if (i >= SDL_NumJoysticks()) {
		unloadController(i);
		return;
	}

	loadController(i);
	updateControllerButtonStates(i);
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

static void updateKeyboardLocalKeyboard(int i) {
	updateKeyStateArraySize(i);
	memcpy(gPrismWindowsInputData.mKeyboards[i].mKeyStates.front().data(), gPrismWindowsInputData.mKeyboards[i].mKeyStatePointer, SDL_NUM_SCANCODES);
	gPrismWindowsInputData.mKeyboards[i].mKeyStates.push_back(gPrismWindowsInputData.mKeyboards[i].mKeyStates.front());
	gPrismWindowsInputData.mKeyboards[i].mConfirmationState.push_back(1);
	gPrismWindowsInputData.mKeyboards[i].mKeyStates.pop_front();
	gPrismWindowsInputData.mKeyboards[i].mConfirmationState.pop_front();
}

static void updateKeyboardRemoteKeyboard(int i) {
	updateKeyStateArraySize(i);
	gPrismWindowsInputData.mKeyboards[i].mKeyStates.push_back(std::vector<Uint8>(SDL_NUM_SCANCODES, 0));
	gPrismWindowsInputData.mKeyboards[i].mConfirmationState.push_back(0);
	gPrismWindowsInputData.mKeyboards[i].mKeyStates.pop_front();
	gPrismWindowsInputData.mKeyboards[i].mConfirmationState.pop_front();
}

static void updateKeyboards()
{
	updateKeyboardLocalKeyboard(PRISM_KEYBOARD_LOCAL);
	if (gPrismWindowsInputData.mUsedKeyboard[0] == PRISM_KEYBOARD_NETPLAY || gPrismWindowsInputData.mUsedKeyboard[1] == PRISM_KEYBOARD_NETPLAY) {
		updateKeyboardRemoteKeyboard(PRISM_KEYBOARD_NETPLAY);
	}
}

void updateInputPlatform() {
	updateKeyboards();
	updateControllers();
}

int hasPressedASingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_A_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_A_PRISM];
	return state;
}

int hasPressedBSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_B_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_B_PRISM];
	return state;
}

int hasPressedXSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_X_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_X_PRISM];
	return state;
}

int hasPressedYSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_Y_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_Y_PRISM];
	return state;
}

int hasPressedLeftSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_LEFT_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_LEFT_PRISM];
	return state;
}

int hasPressedRightSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_RIGHT_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_RIGHT_PRISM];
	return state;
}

int hasPressedUpSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_UP_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_UP_PRISM];
	return state;
}

int hasPressedDownSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_DOWN_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_DOWN_PRISM];
	return state;
}

int hasPressedLSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_L_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_L_PRISM];
	return state;
}

int hasPressedRSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_R_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_R_PRISM];
	return state;
}

int hasPressedStartSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gPrismToSDLKeyboardMapping[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_START_PRISM]].second];
	state |= getButtonStates(i, -gPrismWindowsInputData.mInputDelay)[CONTROLLER_START_PRISM];
	return state;
}

int hasPressedAbortSingle(int i) {
	int state = getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[SDL_SCANCODE_ESCAPE];
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

extern Vector3D correctSDLWindowPosition(const Vector3D& v);

Vector3D getShotPositionSingle(int /*i*/) {
	int x, y;
	SDL_GetMouseState(&x, &y);
	Vector3D ret = Vector3D(x, y, 0);
	return correctSDLWindowPosition(ret);
}


static double getStickNormalizedBinary(int i, int tCodeMinus, int tCodePlus) {
	if (getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[tCodeMinus]) return -1;
	else if (getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[tCodePlus]) return 1;
	else return 0;
}

double getSingleLeftStickNormalizedX(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0;
	}
	else return getStickNormalizedBinary(i, gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_LEFT_PRISM], gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_RIGHT_PRISM]);
}

double getSingleLeftStickNormalizedY(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0;
	}
	else return getStickNormalizedBinary(i, gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_UP_PRISM], gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_DOWN_PRISM]);
}

double getSingleLNormalized(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0;
	}
	else return getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_L_PRISM]];
}

double getSingleRNormalized(int i) {
	if (gPrismWindowsInputData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gPrismWindowsInputData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0;
	}
	else return getKeyStates(gPrismWindowsInputData.mUsedKeyboard[i], -gPrismWindowsInputData.mInputDelay)[gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][CONTROLLER_R_PRISM]];
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
	return getKeyStates(PRISM_KEYBOARD_LOCAL, -gPrismWindowsInputData.mInputDelay)[id];
}

int hasPressedKeyboardKeyFlank(KeyboardKeyPrism tKey) {
	int id = gPrismToSDLKeyboardMapping[tKey].second;
	return !getKeyStates(PRISM_KEYBOARD_LOCAL, -gPrismWindowsInputData.mInputDelay - 1)[id] && getKeyStates(PRISM_KEYBOARD_LOCAL, -gPrismWindowsInputData.mInputDelay)[id];
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
		previousKeyPressed = previousKeyPressed && getKeyStates(PRISM_KEYBOARD_LOCAL, -gPrismWindowsInputData.mInputDelay - 1)[id];
		currentKeyPressed = currentKeyPressed && getKeyStates(PRISM_KEYBOARD_LOCAL, -gPrismWindowsInputData.mInputDelay)[id];
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
	return gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][tTargetButton];
}

void setButtonForKeyboard(int i, ControllerButtonPrism tTargetButton, KeyboardKeyPrism tKeyValue)
{
	gKeys[gPrismWindowsInputData.mUsedKeyboardMapping[i]][tTargetButton] = tKeyValue;
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

int getInputDelay()
{
	return gPrismWindowsInputData.mInputDelay;
}

void setInputDelay(int tInputDelay) {
	gPrismWindowsInputData.mInputDelay = tInputDelay;
	updateKeyStateArraySizes();
	updateControllerButtonStateArraySizes();
}

void setInputBufferSize(int tInputBufferSize)
{
	gPrismWindowsInputData.mInputBuffer = std::max(2, tInputBufferSize);
	updateKeyStateArraySizes();
	updateControllerButtonStateArraySizes();
}

void setInputUsedKeyboardByPlayer(int i, int tKeyboardIndex) {
	gPrismWindowsInputData.mUsedKeyboard[i] = tKeyboardIndex;
}

void setInputUsedKeyboardMappingByPlayer(int i, int tMappingIndex) {
	gPrismWindowsInputData.mUsedKeyboardMapping[i] = tMappingIndex;
}

void gatherWindowsInputStateForPastFrames(int i, size_t pastFrames, std::vector<std::vector<uint8_t>>& tKeyStates, std::vector<std::vector<uint8_t>>& tButtonStates) {
	size_t regularGatherFrames = std::min(pastFrames, gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_LOCAL].mKeyStates.size());
	for (size_t frameIndex = 0; frameIndex < regularGatherFrames; frameIndex++) {
		tKeyStates.push_back(*(gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_LOCAL].mKeyStates.rbegin() + frameIndex));
		tButtonStates.push_back(*(gPrismWindowsInputData.mControllers[i].mButtonStates.rbegin() + frameIndex));
	}

	if (tKeyStates.size() < pastFrames) {
		logWarningFormat("[Input] Unable to gather requested input frame count, availability only %d", regularGatherFrames);
		while (tKeyStates.size() < pastFrames) {
			tKeyStates.push_back(tKeyStates.back());
			tButtonStates.push_back(tButtonStates.back());
		}
	}
}

void gatherWindowsInputStateForLogging(std::vector<uint8_t>& tConfirmationStates, std::vector<std::vector<uint8_t>>& tKeyStates, std::vector<std::vector<uint8_t>>& tButtonStates) {
	for (int i = 0; i < 2; i++)
	{
		tKeyStates.push_back(*(gPrismWindowsInputData.mKeyboards[i].mKeyStates.rbegin()));
		tButtonStates.push_back(*(gPrismWindowsInputData.mControllers[i].mButtonStates.rbegin()));
		tConfirmationStates.push_back(*(gPrismWindowsInputData.mKeyboards[i].mConfirmationState.rbegin()));
	}
}

void setInputForFrameDelta(int i, int tNegativeFrameDelta, const std::vector<std::vector<uint8_t>>& tKeyStates, const std::vector<std::vector<uint8_t>>& tButtonStates)
{
	int baseIndex = -tNegativeFrameDelta;
	for (size_t frameIndex = 0; frameIndex < tKeyStates.size(); frameIndex++) {
		if (baseIndex < 0) {
			baseIndex++;
			continue;
		}
		if (baseIndex >= gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_NETPLAY].mKeyStates.size()) break;
		auto& keyConfirmation = *(gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_NETPLAY].mConfirmationState.rbegin() + baseIndex);
		if (keyConfirmation) continue;

		auto& keyStates = *(gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_NETPLAY].mKeyStates.rbegin() + baseIndex);
		auto& buttonStates = *(gPrismWindowsInputData.mControllers[i].mButtonStates.rbegin() + baseIndex);
		keyStates = tKeyStates[frameIndex];
		keyConfirmation = 1;
		buttonStates = tButtonStates[frameIndex];
		baseIndex++;
	}
}

int isNetplayInputConfirmed() {
	if (gPrismWindowsInputData.mInputDelay >= gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_NETPLAY].mConfirmationState.size()) return 0;
	const auto& confirmationState = *(gPrismWindowsInputData.mKeyboards[PRISM_KEYBOARD_NETPLAY].mConfirmationState.rbegin() + gPrismWindowsInputData.mInputDelay);
	return confirmationState;
}