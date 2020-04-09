#pragma once

#include <cmath>
#include <algorithm>

#ifdef DREAMCAST
#include <kos.h>
#endif

#include "geometry.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#define fclamp(val, mini, maxi) (fmin(fmax(val, mini), maxi))
#define clamp(val, mini, maxi) (std::min(std::max(val, mini), maxi))
#define INF 1000000000

typedef struct {
	double m[4][4];
} Matrix4D;

void setTimeBasedRandomSeed();
void setRandomSeed(unsigned int tSeed);
double randfrom(double tMin, double tMax);
int randfromInteger(int tMin, int tMax);
Position randPositionInGeoRectangle(const GeoRectangle& tRectangle);

double fatan2(double y, double x);

double getLinearInterpolationFactor(double a, double b, double p);
double interpolateLinear(double a, double b, double t);

double fstsqrt(double x);

Matrix4D makeIdentityMatrix4D();
Matrix4D matMult4D(Matrix4D tA, Matrix4D tB);
Matrix4D createScaleMatrix4D(Vector3D tScale);
Matrix4D createTranslationMatrix4D(Vector3D tTranslation);
Matrix4D createRotationZMatrix4D(double tAngle);
Matrix4D createOrthographicProjectionMatrix4D(double tLeft, double tRight, double tUp, double tBottom, double tNear, double tFar);

Position rotateScaleTranslatePositionByMatrix4D(const Matrix4D& tMatrix, const Position& tPosition);