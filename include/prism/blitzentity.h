#pragma once

#include "geometry.h"
#include "blitzcomponent.h"
#include "actorhandler.h"

ActorBlueprint getBlitzEntityHandler();

int addBlitzEntity(const Position& tPos);
void removeBlitzEntity(int tID);

void registerBlitzComponent(int tID, const BlitzComponent& tComponent);

void setBlitzEntityPosition(int tID, const Position& tPos);
void setBlitzEntityPositionX(int tID, double tX);
void setBlitzEntityPositionY(int tID, double tY);
void setBlitzEntityPositionZ(int tID, double tZ);
void addBlitzEntityPosition(int tID, const Vector2D& tPos);
void addBlitzEntityPosition(int tID, const Position& tPos);
void addBlitzEntityPositionX(int tID, double tX);
void addBlitzEntityPositionY(int tID, double tY);
void setBlitzEntityScale2D(int tID, double tScale);
void setBlitzEntityScaleX(int tID, double tScaleX);
void setBlitzEntityScaleY(int tID, double tScaleY);
void setBlitzEntityRotationZ(int tID, double tAngle);
void addBlitzEntityRotationZ(int tID, double tAngle);
void setBlitzEntityParent(int tID, int tParentID);

Position getBlitzEntityPosition(int tID);
double getBlitzEntityPositionX(int tID);
double getBlitzEntityPositionY(int tID);
Vector3D getBlitzEntityScale(int tID);
double getBlitzEntityRotationZ(int tID);
double getBlitzEntityDistance2D(int tID1, int tID2);

Position* getBlitzEntityPositionReference(int tID);
Vector3D* getBlitzEntityScaleReference(int tID);
double* getBlitzEntityRotationZReference(int tID);