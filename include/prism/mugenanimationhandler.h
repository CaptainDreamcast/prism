#pragma once 

#include "actorhandler.h"
#include "geometry.h"
#include "mugenspritefilereader.h"
#include "mugenanimationreader.h"

int addMugenAnimation(MugenAnimation* tStartAnimation, MugenSpriteFile* tSprites, Position tPosition);
void removeMugenAnimation(int tID);

void setMugenAnimationBaseDrawScale(int tID, double tScale);
void setMugenAnimationBasePosition(int tID, Position* tPosition);
void setMugenAnimationScaleReference(int tID, Vector3D* tScale);
void setMugenAnimationAngleReference(int tID, double* tAngle);

void setMugenAnimationCollisionActive(int tID, int tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);
void setMugenAnimationPassiveCollisionActive(int tID, int tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);
void setMugenAnimationAttackCollisionActive(int tID, int tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);

void setMugenAnimationNoLoop(int tID);
void setMugenAnimationCallback(int tID, void(*tFunc)(void*), void* tCaller);

int getMugenAnimationAnimationNumber(int tID);
int getMugenAnimationAnimationStep(int tID);
int getMugenAnimationRemainingAnimationTime(int tID);
void setMugenAnimationFaceDirection(int tID, int tIsFacingRight);
void setMugenAnimationRectangleWidth(int tID, int tWidth);
void setMugenAnimationRectangleHeight(int tID, int tHeight);
void setMugenAnimationCameraPositionReference(int tID, Position* tCameraPosition);
void setMugenAnimationCameraScaleReference(int tID, Position* tCameraScale);
void setMugenAnimationCameraAngleReference(int tID, double* tCameraAngle);

void setMugenAnimationInvisible(int tID);
void setMugenAnimationVisibility(int tID, int tIsVisible);
void setMugenAnimationDrawScale(int tID, Vector3D tScale);
void setMugenAnimationDrawSize(int tID, Vector3D tSize);
void setMugenAnimationDrawAngle(int tID, double tAngle);
void setMugenAnimationColor(int tID, double tR, double tG, double tB);
void setMugenAnimationTransparency(int tID, double tOpacity);
void setMugenAnimationPosition(int tID, Position tPosition);
void setMugenAnimationBlendType(int tID, BlendType tBlendType);
void setMugenAnimationSprites(int tID, MugenSpriteFile* tSprites);
void setMugenAnimationConstraintRectangle(int tID, GeoRectangle tConstraintRectangle);

Position getMugenAnimationPosition(int tID);
int getMugenAnimationIsFacingRight(int tID);
int getMugenAnimationVisibility(int tID);
double getMugenAnimationDrawAngle(int tID);
double getMugenAnimationColorRed(int tID);
double getMugenAnimationColorGreen(int tID);
double getMugenAnimationColorBlue(int tID);

double* getMugenAnimationColorRedReference(int tID);
double* getMugenAnimationColorGreenReference(int tID);
double* getMugenAnimationColorBlueReference(int tID);
double* getMugenAnimationTransparencyReference(int tID);
double* getMugenAnimationScaleXReference(int tID);
double* getMugenAnimationScaleYReference(int tID);
double* getMugenAnimationBaseScaleReference(int tID);
Position* getMugenAnimationPositionReference(int tID);


void changeMugenAnimation(int tID, MugenAnimation* tNewAnimation);
void changeMugenAnimationWithStartStep(int tID, MugenAnimation* tNewAnimation, int tStartStep);

int isStartingMugenAnimationElementWithID(int tID, int tStepID);
int getTimeFromMugenAnimationElement(int tID, int tStep);
int getMugenAnimationElementFromTimeOffset(int tID, int tTime);

void advanceMugenAnimationOneTick(int tID);

void pauseMugenAnimation(int tID);
void unpauseMugenAnimation(int tID);

void pauseMugenAnimationHandler();
void unpauseMugenAnimationHandler();

extern ActorBlueprint MugenAnimationHandler;
ActorBlueprint getMugenAnimationHandlerActorBlueprint();