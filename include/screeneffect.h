#pragma once

#include "animation.h"

typedef void(*ScreenEffectFinishedCB)(void* tCaller);

fup void initScreenEffects();
fup void shutdownScreenEffects();
fup void addVerticalLineFadeIn(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);
fup void addFadeOut(Duration tDuration, ScreenEffectFinishedCB tOptionalCB, void* tCaller);

fup void setScreenBlack();
fup void unsetScreenBlack();