#include "prism/input.h"

#include <stdint.h>

#include <SDL.h>

#include "prism/log.h"
#include "prism/math.h"
#include "prism/clipboardhandler.h"

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

static struct {
	const Uint8* mCurrentKeyStates;
	Controller mControllers[MAXIMUM_CONTROLLER_AMOUNT];
} gData;

static int const gKeys[2][20] = {
	{
		SDL_SCANCODE_A,
		SDL_SCANCODE_S,
		SDL_SCANCODE_Q,
		SDL_SCANCODE_W,
		SDL_SCANCODE_LEFT,
		SDL_SCANCODE_RIGHT,
		SDL_SCANCODE_UP,
		SDL_SCANCODE_DOWN,
		SDL_SCANCODE_E,
		SDL_SCANCODE_D,
		SDL_SCANCODE_RETURN,
		SDL_SCANCODE_ESCAPE,
	},
	{
		SDL_SCANCODE_H,
		SDL_SCANCODE_J,
		SDL_SCANCODE_Y,
		SDL_SCANCODE_U,
		SDL_SCANCODE_4,
		SDL_SCANCODE_6,
		SDL_SCANCODE_8,
		SDL_SCANCODE_2,
		SDL_SCANCODE_I,
		SDL_SCANCODE_K,
		SDL_SCANCODE_3,
		SDL_SCANCODE_ESCAPE,
	},
};

static void loadController(int i) {
	if (gData.mControllers[i].mIsUsingController) return;

	gData.mControllers[i].mController = SDL_GameControllerOpen(i);
	gData.mControllers[i].mHaptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gData.mControllers[i].mController));
	gData.mControllers[i].mIsRumbling = 0;
	gData.mControllers[i].mIsUsingController = 1;
}

static void unloadController(int i) {
	if (!gData.mControllers[i].mIsUsingController) return;
	turnControllerRumbleOffSingle(i);
	if(gData.mControllers[i].mHaptic) SDL_HapticClose(gData.mControllers[i].mHaptic);
	SDL_GameControllerClose(gData.mControllers[i].mController);
	gData.mControllers[i].mController = NULL;
	gData.mControllers[i].mIsUsingController = 0;
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
	if (!gData.mControllers[i].mIsRumbling) return;

	if (handleDurationAndCheckIfOver(&gData.mControllers[i].mRumbleNow, gData.mControllers[i].mRumbleDuration)) {
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

void updateInput() {

	gData.mCurrentKeyStates = SDL_GetKeyboardState(NULL);
	updateControllers();
}



int hasPressedASingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][0]];
	if (gData.mControllers[i].mIsUsingController) state |= SDL_GameControllerGetButton(gData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_A);
	return state;
}

int hasPressedBSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][1]];
	if (gData.mControllers[i].mIsUsingController) state |= SDL_GameControllerGetButton(gData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_B);
	return state;
}

int hasPressedXSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][2]];
	if (gData.mControllers[i].mIsUsingController) state |= SDL_GameControllerGetButton(gData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_X);
	return state;
}

int hasPressedYSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][3]];
	if (gData.mControllers[i].mIsUsingController) state |= SDL_GameControllerGetButton(gData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_Y);
	return state;
}

int hasPressedLeftSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][4]];
	if (gData.mControllers[i].mIsUsingController) {
		double axis = getSingleLeftStickNormalizedX(i);
		state |= (axis < -0.5);
	}
	return state;
}

int hasPressedRightSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][5]];
	if (gData.mControllers[i].mIsUsingController) {
		double axis = getSingleLeftStickNormalizedX(i);
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedUpSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][6]];
	if (gData.mControllers[i].mIsUsingController) {
		double axis = getSingleLeftStickNormalizedY(i);
		state |= (axis < -0.5);
	}
	return state;
}

int hasPressedDownSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][7]];
	if (gData.mControllers[i].mIsUsingController) {
		double axis = getSingleLeftStickNormalizedY(i);
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedLSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][8]];
	if (gData.mControllers[i].mIsUsingController) {
		double axis = getSingleLNormalized(i);
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedRSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][9]];
	if (gData.mControllers[i].mIsUsingController) {
		double axis = getSingleRNormalized(i);
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedStartSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][10]];
	if (gData.mControllers[i].mIsUsingController) state |= SDL_GameControllerGetButton(gData.mControllers[i].mController, SDL_CONTROLLER_BUTTON_START);
	return state;
}

int hasPressedAbortSingle(int i) {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[gKeys[i][11]];
	if (gData.mControllers[i].mIsUsingController) {
		state |= (hasPressedASingle(i) && hasPressedBSingle(i) && hasPressedXSingle(i) && hasPressedYSingle(i) && hasPressedStartSingle(i));
	}
	return state;
}

int hasShotGunSingle(int i)
{
	(void)i;
	uint32_t mask = SDL_GetMouseState(NULL, NULL);
	return mask & SDL_BUTTON(SDL_BUTTON_LEFT); // TODO: fix multiplayer
}

extern Vector3D correctSDLWindowPosition(Vector3D v);

Vector3D getShotPositionSingle(int i) {
	(void)i; // TODO: fix multiplayer

	int x, y;
	SDL_GetMouseState(&x, &y);
	Vector3D ret = makePosition(x, y, 0);
	return correctSDLWindowPosition(ret);
}


static double getStickNormalizedBinary(int tCodeMinus, int tCodePlus) {
	if (gData.mCurrentKeyStates == NULL) return 0;

	if (gData.mCurrentKeyStates[tCodeMinus]) return -1;
	else if (gData.mCurrentKeyStates[tCodePlus]) return 1;
	else return 0;
}

double getSingleLeftStickNormalizedX(int i) {
	if (gData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0;
	}
	else return getStickNormalizedBinary(gKeys[i][4], gKeys[i][5]);
}

double getSingleLeftStickNormalizedY(int i) {
	if (gData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mControllers[i].mController, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0;
	}
	else return getStickNormalizedBinary(gKeys[i][6], gKeys[i][7]);
}

double getSingleLNormalized(int i) {
	if (gData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0;
	}
	else return gData.mCurrentKeyStates[gKeys[i][8]];
}

double getSingleRNormalized(int i) {
	if (gData.mControllers[i].mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mControllers[i].mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0;
	}
	else return gData.mCurrentKeyStates[gKeys[i][9]];
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
	return gData.mControllers[i].mIsUsingController;
}

void addControllerRumbleSingle(int i, Duration tDuration, int tFrequency, double tAmplitude) {
	if (!isUsingControllerSingle(i)) return;
	if (!gData.mControllers[i].mHaptic) return;

	turnControllerRumbleOffSingle(i);

	SDL_HapticEffect* effect = &gData.mControllers[i].mHapticEffect;
	memset(effect, 0, sizeof(SDL_HapticEffect));

	// TODO: fix
	if ((SDL_HapticQuery(gData.mControllers[i].mHaptic) & SDL_HAPTIC_SINE)) {
		effect->type = SDL_HAPTIC_SINE;
	}
	else {
		effect->type = SDL_HAPTIC_LEFTRIGHT;
	}	
	effect->periodic.direction.type = SDL_HAPTIC_POLAR; 
	effect->periodic.direction.dir[0] = 18000; 
	effect->periodic.period = tFrequency; 
	
	effect->periodic.magnitude = (int16_t)(INT16_MAX * fclamp(tAmplitude, 0, 1)); 
	effect->periodic.length = 5000;
	effect->periodic.attack_length = 1000;
	effect->periodic.fade_length = 1000;

	gData.mControllers[i].mHapticEffectID = SDL_HapticNewEffect(gData.mControllers[i].mHaptic, effect);

	SDL_HapticRunEffect(gData.mControllers[i].mHaptic, gData.mControllers[i].mHapticEffectID, 1);

	gData.mControllers[i].mRumbleNow = 0;
	gData.mControllers[i].mRumbleDuration = tDuration;
	gData.mControllers[i].mIsRumbling = 1;
}

void turnControllerRumbleOffSingle(int i) {
	if (!gData.mControllers[i].mIsRumbling) return;

	SDL_HapticDestroyEffect(gData.mControllers[i].mHaptic, gData.mControllers[i].mHapticEffectID);
	gData.mControllers[i].mIsRumbling = 0;
}
