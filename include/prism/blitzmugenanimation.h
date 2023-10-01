#pragma once

#include "blitzcomponent.h"
#include "mugenanimationhandler.h"
#include "actorhandler.h"

ActorBlueprint getBlitzMugenAnimationHandler();

void addBlitzMugenAnimationComponent(int tEntityID, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, int tStartAnimation);
void addBlitzMugenAnimationComponentStatic(int tEntityID, MugenSpriteFile* tSprites, int tSpriteGroup, int tSpriteItem);
MugenAnimationHandlerElement* getBlitzMugenAnimationElement(int tEntityID);
void changeBlitzMugenAnimation(int tEntityID, int tAnimationNumber);
void changeBlitzMugenAnimationWithStartStep(int tEntityID, int tAnimationNumber, int tStep);
void changeBlitzMugenAnimationIfDifferent(int tEntityID, int tAnimationNumber);

void setBlitzMugenAnimationSprites(int tEntityID, MugenSpriteFile* tSprites);
void setBlitzMugenAnimationAnimations(int tEntityID, MugenAnimations* tAnimations);

Position getBlitzMugenAnimationPosition(int tEntityID);
int getBlitzMugenAnimationAnimationNumber(int tEntityID);
int getBlitzMugenAnimationAnimationStep(int tEntityID);
int getBlitzMugenAnimationAnimationStepAmount(int tEntityID);
int getBlitzMugenAnimationAnimationStepDuration(int tEntityID);
int getBlitzMugenAnimationRemainingAnimationTime(int tEntityID);
int getBlitzMugenAnimationTime(int tEntityID);
int getBlitzMugenAnimationDuration(int tEntityID);
Vector3DI getBlitzMugenAnimationSprite(int tEntityID);
int getBlitzMugenAnimationIsFacingRight(int tEntityID);
int getBlitzMugenAnimationIsFacingDown(int tEntityID);
int getBlitzMugenAnimationVisibility(int tEntityID);
Vector2D getBlitzMugenAnimationDrawScale(int tEntityID);

double getBlitzMugenAnimationDrawAngle(int tEntityID);
double getBlitzMugenAnimationColorRed(int tEntityID);
double getBlitzMugenAnimationColorGreen(int tEntityID);
double getBlitzMugenAnimationColorBlue(int tEntityID);

double* getBlitzMugenAnimationColorRedReference(int tEntityID);
double* getBlitzMugenAnimationColorGreenReference(int tEntityID);
double* getBlitzMugenAnimationColorBlueReference(int tEntityID);
double* getBlitzMugenAnimationTransparencyReference(int tEntityID);
double* getBlitzMugenAnimationScaleXReference(int tEntityID);
double* getBlitzMugenAnimationScaleYReference(int tEntityID);
double* getBlitzMugenAnimationBaseScaleReference(int tEntityID);
Position* getBlitzMugenAnimationPositionReference(int tEntityID);
void setBlitzMugenAnimationIndependentOfCamera(int tEntityID);

void setBlitzMugenAnimationTransparency(int tEntityID, double tTransparency);
void setBlitzMugenAnimationFaceDirection(int tEntityID, int tIsFacingRight);
void setBlitzMugenAnimationVerticalFaceDirection(int tEntityID, int tIsFacingDown);
void setBlitzMugenAnimationRectangleWidth(int tEntityID, int tWidth);
void setBlitzMugenAnimationRectangleHeight(int tEntityID, int tHeight);
void setBlitzMugenAnimationPositionX(int tEntityID, double tX);
void setBlitzMugenAnimationPositionY(int tEntityID, double tX);
void setBlitzMugenAnimationVisibility(int tEntityID, int tVisibility);
void setBlitzMugenAnimationColor(int tEntityID, double tR, double tG, double tB);
void setBlitzMugenAnimationBaseDrawScale(int tEntityID, double tScale);
void setBlitzMugenAnimationDrawScale(int tEntityID, const Vector2D& tScale);
void setBlitzMugenAnimationDrawSize(int tEntityID, const Vector2D& tSize);
void setBlitzMugenAnimationAngle(int tEntityID, double tRotation);
void setBlitzMugenAnimationCallback(int tEntityID, void(*tFunc)(void*), void* tCaller);
void setBlitzMugenAnimationInvisible(int tEntityID);
void setBlitzMugenAnimationVisibility(int tEntityID, int tIsVisible);
void setBlitzMugenAnimationBlendType(int tEntityID, BlendType tBlendType);
void setBlitzMugenAnimationConstraintRectangle(int tEntityID, const GeoRectangle2D& tConstraintRectangle);

void setBlitzMugenAnimationNoLoop(int tEntityID);
void setBlitzMugenAnimationCallback(int tEntityID, void(*tFunc)(void*), void* tCaller);

void setBlitzMugenAnimationAnimationStepDuration(int tEntityID, int tDuration);

void pauseBlitzMugenAnimation(int tEntityID);
void unpauseBlitzMugenAnimation(int tEntityID);
int isStartingBlitzMugenAnimationElementWithID(int tEntityID, int tStepID);
int getTimeFromBlitzMugenAnimationElement(int tEntityID, int tStep);
int getBlitzMugenAnimationElementFromTimeOffset(int tEntityID, int tTime);
int isBlitzMugenAnimationTimeOffsetInAnimation(int tEntityID, int tTime);
int getBlitzMugenAnimationTimeWhenStepStarts(int tEntityID, int tStep);

void setBlitzMugenAnimationCollisionDebug(int tEntityID, int tIsActive);