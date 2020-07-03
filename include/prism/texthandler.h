#pragma once

#include "animation.h"
#include "soundeffect.h"
#include "actorhandler.h"

void setupTextHandler();
void shutdownTextHandler();
void updateTextHandler();
void drawHandledTexts();

int addHandledText(const Position& tPosition, const char* tText, int tFont, Color tColor, const Vector3D& tFontSize, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize, Duration tDuration);
int addHandledTextWithBuildup(const Position& tPosition, const char* tText, int tFont, Color tColor, const Vector3D& tFontSize, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize,Duration tDuration, Duration tBuildupDuration);
int addHandledTextWithInfiniteDurationOnOneLine(const Position& tPosition, const char* tText, int tFont, Color tColor, const Vector3D& tFontSize);
void setHandledText(int tID, const char* tText);
void setHandledTextSoundEffects(int tID, const SoundEffectCollection& tSoundEffects);
void setHandledTextPosition(int tID, const Position& tPosition);
void setHandledTextBasePositionReference(int tID, Position* tPosition);
void setHandledTextBuiltUp(int tID);
int isHandledTextBuiltUp(int tID);
void addTextHandlerFont(int tID, const char* tHeaderPath, const char* tTexturePath);

void removeHandledText(int tID);

ActorBlueprint getTextHandler();