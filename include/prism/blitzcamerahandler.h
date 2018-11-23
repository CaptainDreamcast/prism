#pragma once

#include "actorhandler.h"
#include "geometry.h"

ActorBlueprint getBlitzCameraHandler();

int isBlitzCameraHandlerEnabled();
Position* getBlitzCameraHandlerPositionReference();
Position getBlitzCameraHandlerPosition();
void setBlitzCameraHandlerPosition(Position tPos);
void setBlitzCameraHandlerPositionX(double tX);
void setBlitzCameraHandlerPositionY(double tY);

Vector3D* getBlitzCameraHandlerScaleReference();
Vector3D getBlitzCameraHandlerScale();
void setBlitzCameraHandlerScale2D(double tScale);
void setBlitzCameraHandlerScaleX(double tScaleX);
void setBlitzCameraHandlerScaleY(double tScaleY);

double* getBlitzCameraHandlerRotationZReference();
double getBlitzCameraHandlerRotationZ();
void setBlitzCameraHandlerRotationZ(double tAngle);

int getBlitzCameraHandlerEntityID();

void setBlitzCameraHandlerRange(GeoRectangle tRectangle);
void setBlitzCameraPositionBasedOnCenterPoint(Position tCenter);