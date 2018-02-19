#pragma once

#include "geometry.h"
#include "actorhandler.h"

typedef void (*OptionCB)(void* caller);

void setupOptionHandler();
void shutdownOptionHandler();
void disableOptionHandler();
int addOption(Position tPosition, char* tText, OptionCB tCB, void* tCaller);
void removeOption(int tID);
void setOptionTextSize(int tSize);
void setOptionTextBreakSize(int tBreakSize);
void setOptionButtonA();
void setOptionButtonStart();
void updateOptionHandler();
void drawOptionHandler();

ActorBlueprint getOptionHandlerBlueprint();

