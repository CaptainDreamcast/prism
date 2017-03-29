#include "../include/input.h"

#include <SDL.h>

#include "../include/log.h"

static const Uint8* gCurrentKeyStates;

void updateInput() {
	gCurrentKeyStates = SDL_GetKeyboardState(NULL);
}



int hasPressedA() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_A];
}

int hasPressedB() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_S];
}

int hasPressedX() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_Q];
}

int hasPressedY() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_W];
}

int hasPressedLeft() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_LEFT];
}

int hasPressedRight() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_RIGHT];
}

int hasPressedUp() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_UP];
}

int hasPressedDown() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_DOWN];
}

int hasPressedL() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_E];
}

int hasPressedR() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_D];
}

int hasPressedStart() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_1];
}

int hasPressedAbort() {
	if (gCurrentKeyStates == NULL) return 0;
	return gCurrentKeyStates[SDL_SCANCODE_ESCAPE];
}
