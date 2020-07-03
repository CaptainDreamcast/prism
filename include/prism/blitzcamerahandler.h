#pragma once

#include "actorhandler.h"
#include "geometry.h"

ActorBlueprint getBlitzCameraHandler();

int isBlitzCameraHandlerEnabled();
Position* getBlitzCameraHandlerPositionReference();
Position getBlitzCameraHandlerPosition();
void setBlitzCameraHandlerPosition(const Position& tPos);
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

Position2D* getBlitzCameraHandlerEffectPositionReference();
void setBlitzCameraHandlerEffectPositionOffset(const Position2D& tPosition);

int getBlitzCameraHandlerEntityID();

void setBlitzCameraHandlerRange(const GeoRectangle2D& tRectangle);
void setBlitzCameraPositionBasedOnCenterPoint(const Position& tCenter);