#pragma once

#define MAXIMUM_CONTROLLER_AMOUNT 2

#include <stdio.h>

#include "geometry.h"
#include "animation.h"

enum KeyboardKeyPrism : char {
	KEYBOARD_A_PRISM,
	KEYBOARD_B_PRISM,
	KEYBOARD_C_PRISM,
	KEYBOARD_D_PRISM,
	KEYBOARD_E_PRISM,
	KEYBOARD_F_PRISM,
	KEYBOARD_G_PRISM,
	KEYBOARD_H_PRISM,
	KEYBOARD_I_PRISM,
	KEYBOARD_J_PRISM,
	KEYBOARD_K_PRISM,
	KEYBOARD_L_PRISM,
	KEYBOARD_M_PRISM,
	KEYBOARD_N_PRISM,
	KEYBOARD_O_PRISM,
	KEYBOARD_P_PRISM,
	KEYBOARD_Q_PRISM,
	KEYBOARD_R_PRISM,
	KEYBOARD_S_PRISM,
	KEYBOARD_T_PRISM,
	KEYBOARD_U_PRISM,
	KEYBOARD_V_PRISM,
	KEYBOARD_W_PRISM,
	KEYBOARD_X_PRISM,
	KEYBOARD_Y_PRISM,
	KEYBOARD_Z_PRISM,
	KEYBOARD_0_PRISM,
	KEYBOARD_1_PRISM,
	KEYBOARD_2_PRISM,
	KEYBOARD_3_PRISM,
	KEYBOARD_4_PRISM,
	KEYBOARD_5_PRISM,
	KEYBOARD_6_PRISM,
	KEYBOARD_7_PRISM,
	KEYBOARD_8_PRISM,
	KEYBOARD_9_PRISM,
	KEYBOARD_SPACE_PRISM,
	KEYBOARD_LEFT_PRISM,
	KEYBOARD_RIGHT_PRISM,
	KEYBOARD_UP_PRISM,
	KEYBOARD_DOWN_PRISM,
	KEYBOARD_F1_PRISM,
	KEYBOARD_F2_PRISM,
	KEYBOARD_F3_PRISM,
	KEYBOARD_F4_PRISM,
	KEYBOARD_F5_PRISM,
	KEYBOARD_F6_PRISM,
	KEYBOARD_SCROLLLOCK_PRISM,
	KEYBOARD_PAUSE_PRISM,
	KEYBOARD_CARET_PRISM,
	KEYBOARD_CTRL_LEFT_PRISM,
	KEYBOARD_SHIFT_LEFT_PRISM,
	KEYBOARD_RETURN_PRISM,
	KEYBOARD_BACKSPACE_PRISM,
	KEYBOARD_DELETE_PRISM,
	KEYBOARD_PERIOD_PRISM,
	KEYBOARD_SLASH_PRISM,
	KEYBOARD_AMOUNT_PRISM,
};

typedef enum {
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
	CONTROLLER_BUTTON_AMOUNT_PRISM,
} ControllerButtonPrism;

void initInput();
void updateInput();
void resetInputForAllControllers();

int hasPressedA();
int hasPressedB();
int hasPressedX();
int hasPressedY();
int hasPressedLeft();
int hasPressedRight();
int hasPressedUp();
int hasPressedDown();
int hasPressedL();
int hasPressedR();
int hasPressedStart();
int hasPressedAbort();

int hasPressedAFlank();
int hasPressedBFlank();
int hasPressedXFlank();
int hasPressedYFlank();
int hasPressedLeftFlank();
int hasPressedRightFlank();
int hasPressedUpFlank();
int hasPressedDownFlank();
int hasPressedLFlank();
int hasPressedRFlank();
int hasPressedStartFlank();
int hasPressedAbortFlank();

double getLeftStickNormalizedX();
double getLeftStickNormalizedY();
double getLNormalized();
double getRNormalized();

int hasPressedAnyButton();

int hasShotGun();
int hasShotGunFlank();
Vector3D getShotPosition();

int hasPressedASingle(int i);
int hasPressedBSingle(int i);
int hasPressedXSingle(int i);
int hasPressedYSingle(int i);
int hasPressedLeftSingle(int i);
int hasPressedRightSingle(int i);
int hasPressedUpSingle(int i);
int hasPressedDownSingle(int i);
int hasPressedLSingle(int i);
int hasPressedRSingle(int i);
int hasPressedStartSingle(int i);
int hasPressedAbortSingle(int i);

int hasPressedAFlankSingle(int i);
int hasPressedBFlankSingle(int i);
int hasPressedXFlankSingle(int i);
int hasPressedYFlankSingle(int i);
int hasPressedLeftFlankSingle(int i);
int hasPressedRightFlankSingle(int i);
int hasPressedUpFlankSingle(int i);
int hasPressedDownFlankSingle(int i);
int hasPressedLFlankSingle(int i);
int hasPressedRFlankSingle(int i);
int hasPressedStartFlankSingle(int i);
int hasPressedAbortFlankSingle(int i);

double getSingleLeftStickNormalizedX(int i);
double getSingleLeftStickNormalizedY(int i);
double getSingleLNormalized(int i);
double getSingleRNormalized(int i);

int hasShotGunSingle(int i);
int hasShotGunFlankSingle(int i);
Vector3D getShotPositionSingle(int i);

void setMainController(int i);
int getMainController();

void forceMouseCursorToWindow();
void releaseMouseCursorFromWindow();

int isUsingControllerSingle(int i);
int isUsingController();

double getFishingRodIntensity();
double getFishingRodIntensitySingle(int i);

void addControllerRumbleBasic(Duration tDuration);
void addControllerRumble(Duration tDuration, int tFrequency, double tAmplitude);
void turnControllerRumbleOn(int tFrequency, double tAmplitude);
void turnControllerRumbleOff();

void addControllerRumbleBasicSingle(int i, Duration tDuration);
void addControllerRumbleSingle(int i, Duration tDuration, int tFrequency, double tAmplitude);
void turnControllerRumbleOnSingle(int i, int tFrequency, double tAmplitude);
void turnControllerRumbleOffSingle(int i);

int hasPressedRawButton(int i, ControllerButtonPrism tButton);

int hasPressedRawKeyboardKey(KeyboardKeyPrism tKey);
int hasPressedKeyboardKeyFlank(KeyboardKeyPrism tKey);
int hasPressedKeyboardMultipleKeyFlank(int tKeyAmount, ...);

ControllerButtonPrism getButtonForController(int i, ControllerButtonPrism tTargetButton);
void setButtonForController(int i, ControllerButtonPrism tTargetButton, ControllerButtonPrism tButtonValue);
KeyboardKeyPrism getButtonForKeyboard(int i, ControllerButtonPrism tTargetButton);
void setButtonForKeyboard(int i, ControllerButtonPrism tTargetButton, KeyboardKeyPrism tKeyValue);

void setButtonFromUserInputForController(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*) = NULL, void* tCaller = NULL);
void setButtonFromUserInputForKeyboard(int i, ControllerButtonPrism tTargetButton, void(*tOptionalCB)(void*) = NULL, void* tCaller = NULL);
void waitForButtonFromUserInputForController(int i, void(*tCB)(void*, ControllerButtonPrism), void* tCaller = NULL);
void waitForButtonFromUserInputForKeyboard(int i, void(*tCB)(void*, KeyboardKeyPrism), void* tCaller = NULL);
void cancelWaitingForButtonFromUserInput(int i);
