#pragma once

#include "actorhandler.h"

ActorBlueprint getClipboardHandler();

void initClipboardForGame();

void addClipboardLine(char* tLine);
void addClipboardLineFormatString(const char* tFormatString, const char* tParameterString); // expects parameter list in "arg1 , arg2 , arg3" format
void clipf(char* tFormatString, ...);

void clearClipboard();
void setClipboardInvisible();
void setClipboardVisible();