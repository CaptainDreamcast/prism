#pragma once

#include <prism/actorhandler.h>

ActorBlueprint getBlitzClickHandler();

void addBlitzClickComponent(int tEntityID);
void addBlitzClickComponentPassiveAnimationHitbox(int tEntityID);

bool isBlitzEntityClicked(int tEntityID);