#pragma once

#include <cmath>
#include <algorithm>

#ifdef DREAMCAST
#include <kos.h>
#endif

#include "geometry.h"

namespace prism {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#define fclamp(val, mini, maxi) (std::fmin(std::fmax(val, mini), maxi))
#define clamp(val, mini, maxi) (std::min(std::max(val, mini), maxi))
#define INF 1000000000

typedef struct {
	float m[4][4];
} Matrix4D;

void setTimeBasedRandomSeed();
void setRandomSeed(unsigned int tSeed);
unsigned int getRandomState();
void setRandomState(unsigned int tState);
unsigned long long getRandomCallAmount();
float randfrom(float tMin, float tMax);
int randfromInteger(int tMin, int tMax);
Position randPositionInGeoRectangle(const GeoRectangle& tRectangle);

float fatan2(float y, float x);

float getLinearInterpolationFactor(float a, float b, float p);
float interpolateLinear(float a, float b, float t);

float fstsqrt(float x);

Matrix4D makeIdentityMatrix4D();
Matrix4D matMult4D(const Matrix4D& tA, const Matrix4D& tB);
Matrix4D createScaleMatrix4D(const Vector2D& tScale);
Matrix4D createScaleMatrix4D(const Vector3D& tScale);
Matrix4D createTranslationMatrix4D(const Vector2D& tTranslation);
Matrix4D createTranslationMatrix4D(const Vector3D& tTranslation);
Matrix4D createRotationZMatrix4D(float tAngle);
Matrix4D createOrthographicProjectionMatrix4D(float tLeft, float tRight, float tUp, float tBottom, float tNear, float tFar);

Position rotateScaleTranslatePositionByMatrix4D(const Matrix4D& tMatrix, const Position& tPosition);

}