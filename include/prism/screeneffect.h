#pragma once

#include "animation.h"
#include "texture.h"
#include "actorhandler.h"
#include "tweening.h"

namespace prism {

typedef void(*ScreenEffectFinishedCB)(void* tCaller);

void initScreenEffects();
void shutdownScreenEffects();

ActorBlueprint getScreenEffectHandler();

void addFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller = nullptr);
void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller = nullptr);
void skipFadeIn();
void addFadeOut(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller = nullptr);
void setFadeColor(Color tColor);
void setFadeColorRGB(float r, float g, float b);
void setScreenEffectZ(float tZ);

void setScreenTint(Color tColor, float tAlpha);
void setScreenTintRGBA(float tR, float tG, float tB, float tAlpha);
void removeScreenTint();

void tweenScreenTint(Color tColor, float tStartAlpha, float tEndAlpha, Duration tDuration, TweeningFunction tFunc = linearTweeningFunction);
void tweenScreenTintRGBA(float tR, float tG, float tB, float tStartAlpha, float tEndAlpha, Duration tDuration, TweeningFunction tFunc = linearTweeningFunction);
void setScreenTintZ(float tZ);

// Fire-and-forget convenience screen tint functions
void addDamageFlash(Duration tDuration = 12);
void addDamageFlashColored(Color tColor, float tStartAlpha, Duration tDuration);

void drawColoredRectangle(const GeoRectangle& tRect, Color tColor);
void drawColoredHorizontalLine(const Position& tA, const Position& tB, Color tColor);
void drawColoredPoint(const Position& tPoint, Color tColor);

void setScreenColor(Color tColor);
void setScreenBackgroundColorRGB(float tR, float tG, float tB);
void unsetScreenColor();
void setScreenBlack();
void unsetScreenBlack();

void setScreenWhite();
void unsetScreenWhite();

TextureData getEmptyWhiteTexture();
TextureData* getEmptyWhiteTextureReference();

#ifdef _WIN32
void imguiScreenEffects();
#endif

}