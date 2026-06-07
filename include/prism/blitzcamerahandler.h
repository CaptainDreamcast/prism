#pragma once

#include "actorhandler.h"
#include "geometry.h"

namespace prism { 

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

const GeoRectangle2D& getBlitzCameraHandlerRange();
void setBlitzCameraHandlerRange(const GeoRectangle2D& tRectangle);
void setBlitzCameraPositionBasedOnCenterPoint(const Position& tCenter);

void setBlitzCameraScreenShake(int tDuration, double tFrequency, int tAmplitude, double tPhaseOffset);
void setBlitzCameraScreenShakeDefault();

void setBlitzCameraZoom(const Vector2D& tPosition, double tZoomFactor);
void setBlitzCameraZoom(const GeoRectangle2D& tZoomArea);

void imguiBlitzCameraHandler();

}