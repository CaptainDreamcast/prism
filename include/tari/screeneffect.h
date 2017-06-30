#pragma once

#include "animation.h"

typedef void(*ScreenEffectFinishedCB)(void* tCaller);

fup void initScreenEffects();
fup void shutdownScreenEffects();
fup void addFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);
fup void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);
fup void addFadeOut(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);

fup void drawColoredRectangle(GeoRectangle tRect, Color tColor);

fup void setScreenColor(Color tColor);
fup void unsetScreenColor();
fup void setScreenBlack();
fup void unsetScreenBlack();

fup void setScreenWhite();
fup void unsetScreenWhite();
