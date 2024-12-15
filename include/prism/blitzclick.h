#pragma once

#include <prism/actorhandler.h>

namespace prism {

ActorBlueprint getBlitzClickHandler();

void addBlitzClickComponent(int tEntityID);
void addBlitzClickComponentPassiveAnimationHitbox(int tEntityID);

bool isBlitzEntityClicked(int tEntityID);

}