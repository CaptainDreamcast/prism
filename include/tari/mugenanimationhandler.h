#pragma once 

#include <tari/actorhandler.h>
#include <tari/geometry.h>
#include <tari/mugenspritefilereader.h>
#include <tari/mugenanimationreader.h>

int addMugenAnimation(MugenAnimation* tStartAnimation, MugenSpriteFile* tSprites, Position tPosition);
void removeMugenAnimation(int tID);

void setMugenAnimationBaseDrawScale(int tID, double tScale);
void setMugenAnimationBasePosition(int tID, Position* tPosition);

void setMugenAnimationCollisionActive(int tID, int tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);
void setMugenAnimationNoLoop(int tID);
void setMugenAnimationCallback(int tID, void(*tFunc)(void*), void* tCaller);

int getMugenAnimationAnimationNumber(int tID);
int getMugenAnimationRemainingAnimationTime(int tID);
void setMugenAnimationFaceDirection(int tID, int tIsFacingRight);
void setMugenAnimationRectangleWidth(int tID, int tWidth);
void setMugenAnimationRectangleHeight(int tID, int tHeight);
void setMugenAnimationCameraPositionReference(int tID, Position* tCameraPosition);
void setMugenAnimationInvisible(int tID);
void setMugenAnimationDrawScale(int tID, Vector3D tScale);
void setMugenAnimationDrawSize(int tID, Vector3D tSize);
void setMugenAnimationDrawAngle(int tID, double tAngle);
void addMugenAnimationDrawAngle(int tID, double tAngle);
void setMugenAnimationColor(int tID, double tR, double tG, double tB);
void setMugenAnimationTransparency(int tID, double tOpacity);
void setMugenAnimationPosition(int tID, Position tPosition);
void setMugenAnimationBlendType(int tID, BlendType tBlendType);

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

ActorBlueprint getMugenAnimationHandlerActorBlueprint();