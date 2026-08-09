#pragma once

#include "actorhandler.h"
#include "geometry.h"

namespace prism { 

ActorBlueprint getBlitzCameraHandler();

int isBlitzCameraHandlerEnabled();
Position* getBlitzCameraHandlerPositionReference();
Position getBlitzCameraHandlerPosition();
void setBlitzCameraHandlerPosition(const Position& tPos);
void setBlitzCameraHandlerPositionX(float tX);
void setBlitzCameraHandlerPositionY(float tY);

Vector3D* getBlitzCameraHandlerScaleReference();
Vector3D getBlitzCameraHandlerScale();
void setBlitzCameraHandlerScale2D(float tScale);
void setBlitzCameraHandlerScaleX(float tScaleX);
void setBlitzCameraHandlerScaleY(float tScaleY);

float* getBlitzCameraHandlerRotationZReference();
float getBlitzCameraHandlerRotationZ();
void setBlitzCameraHandlerRotationZ(float tAngle);

Position2D* getBlitzCameraHandlerEffectPositionReference();
void setBlitzCameraHandlerEffectPositionOffset(const Position2D& tPosition);

int getBlitzCameraHandlerEntityID();

const GeoRectangle2D& getBlitzCameraHandlerRange();
void setBlitzCameraHandlerRange(const GeoRectangle2D& tRectangle);
void setBlitzCameraPositionBasedOnCenterPoint(const Position& tCenter);

void setBlitzCameraScreenShake(int tDuration, float tFrequency, int tAmplitude, float tPhaseOffset);
void setBlitzCameraScreenShakeDefault();

void setBlitzCameraZoom(const Vector2D& tPosition, float tZoomFactor);
void setBlitzCameraZoom(const GeoRectangle2D& tZoomArea);

void imguiBlitzCameraHandler();

}