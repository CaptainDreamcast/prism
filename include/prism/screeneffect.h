#pragma once

#include "animation.h"
#include "texture.h"

typedef void(*ScreenEffectFinishedCB)(void* tCaller);

void initScreenEffects();
void shutdownScreenEffects();
void addFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);
void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);
void addFadeOut(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);
void setFadeColor(Color tColor);
void setFadeColorRGB(double r, double g, double b);
void setScreenEffectZ(double tZ);

void drawColoredRectangle(GeoRectangle tRect, Color tColor);
void drawColoredHorizontalLine(Position tA, Position tB, Color tColor);
void drawColoredPoint(Position tPoint, Color tColor);

void setScreenColor(Color tColor);
void setScreenBackgroundColorRGB(double tR, double tG, double tB);
void unsetScreenColor();
void setScreenBlack();
void unsetScreenBlack();

void setScreenWhite();
void unsetScreenWhite();

TextureData getEmptyWhiteTexture();