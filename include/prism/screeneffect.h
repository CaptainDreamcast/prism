#pragma once

#include "animation.h"
#include "texture.h"
#include "actorhandler.h"

typedef void(*ScreenEffectFinishedCB)(void* tCaller);

void initScreenEffects();
void shutdownScreenEffects();

ActorBlueprint getScreenEffectHandler();

void addFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller = nullptr);
void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller = nullptr);
void skipFadeIn();
void addFadeOut(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller = nullptr);
void setFadeColor(Color tColor);
void setFadeColorRGB(double r, double g, double b);
void setScreenEffectZ(double tZ);

void drawColoredRectangle(const GeoRectangle& tRect, Color tColor);
void drawColoredHorizontalLine(const Position& tA, const Position& tB, Color tColor);
void drawColoredPoint(const Position& tPoint, Color tColor);

void setScreenColor(Color tColor);
void setScreenBackgroundColorRGB(double tR, double tG, double tB);
void unsetScreenColor();
void setScreenBlack();
void unsetScreenBlack();

void setScreenWhite();
void unsetScreenWhite();

TextureData getEmptyWhiteTexture();
TextureData* getEmptyWhiteTextureReference();
