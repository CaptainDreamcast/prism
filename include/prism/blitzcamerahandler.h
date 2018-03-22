#pragma once

#include "actorhandler.h"
#include "geometry.h"

extern ActorBlueprint BlitzCameraHandler;

int isBlitzCameraHandlerEnabled();
Position* getBlitzCameraHandlerPositionReference();
Position getBlitzCameraHandlerPosition();
void setBlitzCameraHandlerPosition(Position tPos);
void setBlitzCameraHandlerPositionX(double tX);
void setBlitzCameraHandlerPositionY(double  tY);

int getBlitzCameraHandlerEntityID();