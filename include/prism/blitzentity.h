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
Position getBlitzEntityPosition(int tID);
Position* getBlitzEntityPositionReference(int tID);
