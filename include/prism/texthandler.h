#pragma once

#include "animation.h"
#include "soundeffect.h"

void setupTextHandler();
void shutdownTextHandler();
void updateTextHandler();
void drawHandledTexts();

int addHandledText(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize, Duration tDuration);
int addHandledTextWithBuildup(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize,Duration tDuration, Duration tBuildupDuration);
int addHandledTextWithInfiniteDurationOnOneLine(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize);
void setHandledText(int tID, char* tText);
void setHandledTextSoundEffects(int tID, SoundEffectCollection tSoundEffects);
void setHandledTextPosition(int tID, Position tPosition);
void setHandledTextBasePositionReference(int tID, Position* tPosition);
void setHandledTextBuiltUp(int tID);
int isHandledTextBuiltUp(int tID);

void removeHandledText(int tID);

