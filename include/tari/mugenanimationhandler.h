#pragma once 

#include <tari/actorhandler.h>
#include <tari/geometry.h>
#include <tari/mugenspritefilereader.h>
#include <tari/mugenanimationreader.h>

fup int addMugenAnimation(MugenAnimation* tStartAnimation, MugenSpriteFile* tSprites, Position tPosition);
fup void removeMugenAnimation(int tID);

fup void setMugenAnimationBaseDrawScale(int tID, double tScale);
fup void setMugenAnimationBasePosition(int tID, Position* tPosition);

fup void setMugenAnimationCollisionActive(int tID, int tCollisionList, void(*tFunc)(void*, void*), void* tCaller, void* tCollisionData);
fup void setMugenAnimationNoLoop(int tID);

fup int getMugenAnimationAnimationNumber(int tID);
fup int getMugenAnimationRemainingAnimationTime(int tID);
fup void setMugenAnimationFaceDirection(int tID, int tIsFacingRight);
fup void setMugenAnimationRectangleWidth(int tID, int tWidth);
fup void setMugenAnimationCameraPositionReference(int tID, Position* tCameraPosition);
fup void setMugenAnimationInvisible(int tID);
fup void setMugenAnimationDrawScale(int tID, Vector3D tScale);
fup void setMugenAnimationDrawAngle(int tID, double tAngle);
fup void setMugenAnimationColor(int tID, double tR, double tG, double tB);

fup void changeMugenAnimation(int tID, MugenAnimation* tNewAnimation);
fup void changeMugenAnimationWithStartStep(int tID, MugenAnimation* tNewAnimation, int tStartStep);

fup int isStartingMugenAnimationElementWithID(int tID, int tStepID);
fup int getTimeFromMugenAnimationElement(int tID, int tStep);
fup int getMugenAnimationElementFromTimeOffset(int tID, int tTime);

fup void pauseMugenAnimation(int tID);
fup void unpauseMugenAnimation(int tID);

fup void pauseMugenAnimationHandler();
fup void unpauseMugenAnimationHandler();

fup ActorBlueprint getMugenAnimationHandlerActorBlueprint();