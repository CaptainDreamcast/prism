#pragma once

#include "geometry.h"
#include "blitzcomponent.h"
#include "actorhandler.h"

extern ActorBlueprint BlitzEntityHandler;

int addBlitzEntity(Position tPos);
void removeBlitzEntity(int tID);

void registerBlitzComponent(int tID, BlitzComponent tComponent);

void setBlitzEntityPosition(int tID, Position tPos);
void setBlitzEntityPositionX(int tID, double tX);
void setBlitzEntityPositionY(int tID, double tY);
void setBlitzEntityScale2D(int tID, double tScale);
void setBlitzEntityScaleX(int tID, double tScaleX);
void setBlitzEntityScaleY(int tID, double tScaleY);
void setBlitzEntityRotationZ(int tID, double tAngle);
void setBlitzEntityParent(int tID, int tParentID);

Position getBlitzEntityPosition(int tID);
double getBlitzEntityPositionX(int tID);
double getBlitzEntityPositionY(int tID);
Vector3D getBlitzEntityScale(int tID);
double getBlitzEntityRotationZ(int tID);

Position* getBlitzEntityPositionReference(int tID);
Vector3D* getBlitzEntityScaleReference(int tID);
double* getBlitzEntityRotationZReference(int tID);