#pragma once

#include "blitzcomponent.h"
#include "mugenanimationhandler.h"
#include "actorhandler.h"

extern ActorBlueprint BlitzMugenAnimationHandler;

void addBlitzMugenAnimationComponent(int tEntityID, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, int tStartAnimation);
void addBlitzMugenAnimationComponentStatic(int tEntityID, MugenSpriteFile* tSprites, int tSpriteGroup, int tSpriteItem);
int getBlitzMugenAnimationID(int tEntityID);