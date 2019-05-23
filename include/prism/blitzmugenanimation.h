#pragma once

#include "blitzcomponent.h"
#include "mugenanimationhandler.h"
#include "actorhandler.h"

ActorBlueprint getBlitzMugenAnimationHandler();

void addBlitzMugenAnimationComponent(int tEntityID, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, int tStartAnimation);
void addBlitzMugenAnimationComponentStatic(int tEntityID, MugenSpriteFile* tSprites, int tSpriteGroup, int tSpriteItem);
int getBlitzMugenAnimationID(int tEntityID);
void changeBlitzMugenAnimation(int tEntityID, int tAnimationNumber);
void changeBlitzMugenAnimationWithStartStep(int tEntityID, int tAnimationNumber, int tStep);
void changeBlitzMugenAnimationIfDifferent(int tEntityID, int tAnimationNumber);
int getBlitzMugenAnimationAnimationNumber(int tEntityID);

void setBlitzMugenAnimationTransparency(int tEntityID, double tTransparency);
void setBlitzMugenAnimationFaceDirection(int tEntityID, int tIsFacingRight);
void setBlitzMugenAnimationVerticalFaceDirection(int tEntityID, int tIsFacingDown);
void setBlitzMugenAnimationPositionX(int tEntityID, double tX);
void setBlitzMugenAnimationPositionY(int tEntityID, double tX);
void setBlitzMugenAnimationVisibility(int tEntityID, int tVisibility);
void setBlitzMugenAnimationColor(int tEntityID, double tR, double tG, double tB);
void setBlitzMugenAnimationBaseDrawScale(int tEntityID, double tScale);
void setBlitzMugenAnimationCallback(int tEntityID, void(*tFunc)(void*), void* tCaller);
