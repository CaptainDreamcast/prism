#pragma once

#define MAXIMUM_CONTROLLER_AMOUNT 2

#include <stdio.h>

#include "geometry.h"
#include "animation.h"

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


typedef enum {
	KEYBOARD_C_PRISM,
	KEYBOARD_D_PRISM,
	KEYBOARD_I_PRISM,
	KEYBOARD_L_PRISM,
	KEYBOARD_S_PRISM,
	KEYBOARD_V_PRISM,
	KEYBOARD_SPACE_PRISM,
	KEYBOARD_F1_PRISM,
	KEYBOARD_F2_PRISM,
	KEYBOARD_F3_PRISM,
	KEYBOARD_F4_PRISM,
	KEYBOARD_F5_PRISM,
	KEYBOARD_F6_PRISM,
	KEYBOARD_SCROLLLOCK_PRISM,
	KEYBOARD_PAUSE_PRISM,
	KEYBOARD_CTRL_LEFT_PRISM,
	KEYBOARD_SHIFT_LEFT_PRISM,
	KEYBOARD_AMOUNT_PRISM,
} KeyboardKeyPrism;

int hasPressedKeyboardKeyFlank(KeyboardKeyPrism tKey);
int hasPressedKeyboardMultipleKeyFlank(int tKeyAmount, ...);