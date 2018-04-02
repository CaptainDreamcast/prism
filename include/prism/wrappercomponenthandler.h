#pragma once

// TODO: rewrite with custom actor struct
#include "actorhandler.h"

void setupWrapperComponentHandler();
void updateWrapperComponentHandler();
void drawWrapperComponentHandler();
void shutdownWrapperComponentHandler();

void addWrapperComponent(ActorBlueprint tComponentBlueprint);