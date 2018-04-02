#pragma once

#include "blitzcomponent.h"
#include "mugenanimationhandler.h"
#include "actorhandler.h"

extern ActorBlueprint BlitzMugenAnimationHandler;

void addBlitzMugenAnimationComponent(int tEntityID, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, int tStartAnimation);
void addBlitzMugenAnimationComponentStatic(int tEntityID, MugenSpriteFile* tSprites, int tSpriteGroup, int tSpriteItem);
int getBlitzMugenAnimationID(int tEntityID);
void changeBlitzMugenAnimation(int tEntityID, int tAnimationNumber);
void changeBlitzMugenAnimationWithStartStep(int tEntityID, int tAnimationNumber, int tStep);
void changeBlitzMugenAnimationIfDifferent(int tEntityID, int tAnimationNumber);
int getBlitzMugenAnimationAnimationNumber(int tEntityID);

void setBlitzMugenAnimationTransparency(int tEntityID, double tTransparency);
void setBlitzMugenAnimationFaceDirection(int tEntityID, int tIsFacingRight);