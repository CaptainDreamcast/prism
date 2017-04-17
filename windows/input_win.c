#include "../include/input.h"

#include <SDL.h>

#include "../include/log.h"


static struct {
	const Uint8* mCurrentKeyStates;

	int mIsUsingController;
	SDL_GameController* mController;

} gData;

static void loadController() {
	if (gData.mIsUsingController) return;

	gData.mController = SDL_GameControllerOpen(0);
	gData.mIsUsingController = 1;
}

static void unloadController() {
	if (!gData.mIsUsingController) return;

	SDL_GameControllerClose(gData.mController);
	gData.mController = NULL;
	gData.mIsUsingController = 0;
}

static void updateControllerInput() {
	if (SDL_NumJoysticks() == 0) {
		unloadController();
		return;
	}


	loadController();

}

void updateInput() {
	
	gData.mCurrentKeyStates = SDL_GetKeyboardState(NULL);
	updateControllerInput();
}



int hasPressedA() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_A];
	if (gData.mIsUsingController) state |= SDL_GameControllerGetButton(gData.mController, SDL_CONTROLLER_BUTTON_A);
	return state;
}

int hasPressedB() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_S];
	if (gData.mIsUsingController) state |= SDL_GameControllerGetButton(gData.mController, SDL_CONTROLLER_BUTTON_B);
	return state;
}

int hasPressedX() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_Q];
	if (gData.mIsUsingController) state |= SDL_GameControllerGetButton(gData.mController, SDL_CONTROLLER_BUTTON_X);
	return state;
}

int hasPressedY() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_W];
	if (gData.mIsUsingController) state |= SDL_GameControllerGetButton(gData.mController, SDL_CONTROLLER_BUTTON_Y);
	return state;
}

int hasPressedLeft() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_LEFT];
	if (gData.mIsUsingController) {
		double axis = getLeftStickNormalizedX();
		state |= (axis < -0.5);
	}
	return state;
}

int hasPressedRight() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_RIGHT];
	if (gData.mIsUsingController) {
		double axis = getLeftStickNormalizedX();
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedUp() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_UP];
	if (gData.mIsUsingController) {
		double axis = getLeftStickNormalizedY();
		state |= (axis < -0.5);
	}
	return state;
}

int hasPressedDown() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_DOWN];
	if (gData.mIsUsingController) {
		double axis = getLeftStickNormalizedY();
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedL() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_E];
	if (gData.mIsUsingController) {
		double axis = getLNormalized();
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedR() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_D];
	if (gData.mIsUsingController) {
		double axis = getRNormalized();
		state |= (axis > 0.5);
	}
	return state;
}

int hasPressedStart() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_1];
	if (gData.mIsUsingController) state |= SDL_GameControllerGetButton(gData.mController, SDL_CONTROLLER_BUTTON_START);
	return state;
}

int hasPressedAbort() {
	if (gData.mCurrentKeyStates == NULL) return 0;
	int state = gData.mCurrentKeyStates[SDL_SCANCODE_ESCAPE];
	if (gData.mIsUsingController) {
		state |= (hasPressedA() && hasPressedB() && hasPressedX() && hasPressedY() && hasPressedStart());
	}
	return state;
}

static double getStickNormalizedBinary(int tCodeMinus, int tCodePlus) {
	if (gData.mCurrentKeyStates == NULL) return 0;

	if (gData.mCurrentKeyStates[tCodeMinus]) return -1;
	else if (gData.mCurrentKeyStates[tCodePlus]) return 1;
	else return 0;
}

double getLeftStickNormalizedX() {
	if (gData.mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mController, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0;
	}
	else return getStickNormalizedBinary(SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT);
}

double getLeftStickNormalizedY() {
	if (gData.mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mController, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0;
	}
	else return getStickNormalizedBinary(SDL_SCANCODE_UP, SDL_SCANCODE_DOWN);
}

double getLNormalized() {
	if (gData.mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0;
	}
	else return gData.mCurrentKeyStates[SDL_SCANCODE_E];
}

double getRNormalized() {
	if (gData.mIsUsingController) {
		return SDL_GameControllerGetAxis(gData.mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0;
	}
	else return gData.mCurrentKeyStates[SDL_SCANCODE_D];
}