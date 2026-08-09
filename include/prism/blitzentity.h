#pragma once

#include "geometry.h"
#include "blitzcomponent.h"
#include "actorhandler.h"

namespace prism {

ActorBlueprint getBlitzEntityHandler();

int addBlitzEntity(const Position& tPos);
void removeBlitzEntity(int tID);

void registerBlitzComponent(int tID, const BlitzComponent& tComponent);

void setBlitzEntityPosition(int tID, const Position& tPos);
void setBlitzEntityPositionX(int tID, float tX);
void setBlitzEntityPositionY(int tID, float tY);
void setBlitzEntityPositionZ(int tID, float tZ);
void setBlitzEntityPositionXY(int tID, const Vector2D& tPos);
void addBlitzEntityPosition(int tID, const Vector2D& tPos);
void addBlitzEntityPosition(int tID, const Position& tPos);
void addBlitzEntityPositionX(int tID, float tX);
void addBlitzEntityPositionY(int tID, float tY);
void setBlitzEntityScale2D(int tID, float tScale);
void setBlitzEntityScaleX(int tID, float tScaleX);
void setBlitzEntityScaleY(int tID, float tScaleY);
void setBlitzEntityRotationZ(int tID, float tAngle);
void addBlitzEntityRotationZ(int tID, float tAngle);
void setBlitzEntityParent(int tID, int tParentID);

Position getBlitzEntityPosition(int tID);
float getBlitzEntityPositionX(int tID);
float getBlitzEntityPositionY(int tID);
float getBlitzEntityPositionZ(int tID);
Vector3D getBlitzEntityScale(int tID);
float getBlitzEntityRotationZ(int tID);
float getBlitzEntityDistance2D(int tID1, int tID2);

Position* getBlitzEntityPositionReference(int tID);
Vector3D* getBlitzEntityScaleReference(int tID);
float* getBlitzEntityRotationZReference(int tID);

void imguiBlitzEntityHandler();

}