#include "prism/input.h"

namespace prism {

	void initInput() {}
	void updateInputPlatform() {}

	int hasPressedASingle(int /*i*/) { return 0; }
	int hasPressedBSingle(int /*i*/) { return 0; }
	int hasPressedXSingle(int /*i*/) { return 0; }
	int hasPressedYSingle(int /*i*/) { return 0; }
	int hasPressedLeftSingle(int /*i*/) { return 0; }
	int hasPressedRightSingle(int /*i*/) { return 0; }
	int hasPressedUpSingle(int /*i*/) { return 0; }
	int hasPressedDownSingle(int /*i*/) { return 0; }
	int hasPressedLSingle(int /*i*/) { return 0; }
	int hasPressedRSingle(int /*i*/) { return 0; }
	int hasPressedStartSingle(int /*i*/) { return 0; }
	int hasPressedAbortSingle(int /*i*/) { return 0; }

	float getSingleLeftStickNormalizedX(int /*i*/) { return 0; }
	float getSingleLeftStickNormalizedY(int /*i*/) { return 0; }
	float getSingleLNormalized(int /*i*/) { return 0; }
	float getSingleRNormalized(int /*i*/) { return 0; }

	int hasShotGunSingle(int /*i*/) { return 0; }
	Vector3D getShotPositionSingle(int /*i*/) { return Vector3D(0, 0, 0); }

	int isUsingControllerSingle(int /*i*/) { return 0; }
	void addControllerRumbleSingle(int /*i*/, Duration /*tDuration*/, int /*tFrequency*/, float /*tAmplitude*/) {}
	void turnControllerRumbleOffSingle(int /*i*/) {}

	int hasPressedRawButton(int /*i*/, ControllerButtonPrism /*tButton*/) { return 0; }
	int hasPressedRawKeyboardKey(KeyboardKeyPrism /*tKey*/) { return 0; }
	int hasPressedKeyboardKeyFlank(KeyboardKeyPrism /*tKey*/) { return 0; }
	int hasPressedKeyboardMultipleKeyFlank(int /*tKeyAmount*/, ...) { return 0; }

	ControllerButtonPrism getButtonForController(int /*i*/, ControllerButtonPrism tTargetButton) { return tTargetButton; }
	void setButtonForController(int /*i*/, ControllerButtonPrism /*tTargetButton*/, ControllerButtonPrism /*tButtonValue*/) {}
	KeyboardKeyPrism getButtonForKeyboard(int /*i*/, ControllerButtonPrism /*tTargetButton*/) { return KEYBOARD_A_PRISM; }
	void setButtonForKeyboard(int /*i*/, ControllerButtonPrism /*tTargetButton*/, KeyboardKeyPrism /*tKeyValue*/) {}

	void waitForCharacterFromUserInput(int /*i*/, void(*/*tCB*/)(void*, const std::string&), void* /*tCaller*/) {}
	void cancelWaitingForCharacterFromUserInput(int /*i*/) {}

	void setInputDelay(int /*tInputDelay*/) {}

	bool isMouseInRectangle(const GeoRectangle2D& /*tRectangle*/) { return false; }
	bool hasPressedMouseLeftSingle(int /*i*/) { return false; }
	bool hasPressedMouseRightSingle(int /*i*/) { return false; }

}
