#pragma once

#include <string>

#include <prism/actorhandler.h>

int isInDevelopMode();

void initDebug();
ActorBlueprint getPrismDebug();

void setPrismDebugUpdateStartTime();
void setPrismDebugDrawingStartTime();
void setPrismDebugWaitingStartTime();

void addPrismDebugConsoleCommand(std::string tCommand, std::string(*tCB)(void* tCaller, std::string tCommandInput), void* tCaller = NULL);
void submitToPrismDebugConsole(std::string tText);