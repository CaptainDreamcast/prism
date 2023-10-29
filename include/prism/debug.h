#pragma once

#include <string>
#include <functional>

#include "prism/actorhandler.h"
#include "prism/log.h"

int isInDevelopMode();
void setDevelopMode();

void initDebug();
ActorBlueprint getPrismDebug();

void setPrismDebugUpdateStartTime();
void setPrismDebugDrawingStartTime();
void setPrismDebugWaitingStartTime();
void setPrismDebugDropFrameCounter(int tDropFrameCounter);
int getPrismDebugSideDisplayVisibility();
void setPrismDebugSideDisplayVisibility(int tIsVisible);
void togglePrismDebugSideDisplayVisibility();

int isPrismDebugConsoleVisible();
void addPrismDebugConsoleCommand(const std::string& tCommand, std::string(*tCB)(void* tCaller, const std::string& tCommandInput), void* tCaller = NULL);
void submitToPrismDebugConsole(const std::string& tText);