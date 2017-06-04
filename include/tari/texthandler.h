#pragma once

#include "animation.h"

fup void setupTextHandler();
fup void shutdownTextHandler();
fup void updateTextHandler();
fup void drawHandledTexts();

fup int addHandledText(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize, Duration tDuration);
fup int addHandledTextWithBuildup(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize,Duration tDuration, Duration tBuildupSpeed);
fup int addHandledTextWithInfiniteDurationOnOneLine(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize);
fup void setHandledText(int tID, char* tText);
fup void removeHandledText(int tID);

