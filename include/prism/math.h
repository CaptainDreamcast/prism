#pragma once

#include <cmath>
#include <algorithm>

#include "geometry.h"

#ifdef DREAMCAST

#include <kos.h>

#define fmin	min
#define fmax	max
#define fabs 	abs
// TODO: sort out the math header stuff

#define M_PI 3.14159265358979323846 // TODO: fix for #Dreamcast
#define M_E 2.71828182845904523536

#elif defined __EMSCRIPTEN__
#endif

#define fclamp(val, mini, maxi) (fmin(fmax(val, mini), maxi))
#define clamp(val, mini, maxi) (std::min(std::max(val, mini), maxi))
#define INF 1000000000

typedef struct {
	double m[4][4];
} Matrix4D;

double randfrom(double tMin, double tMax);
int randfromInteger(int tMin, int tMax);

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

