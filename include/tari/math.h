#ifndef TARI_MATH
#define TARI_MATH

#include "common/header.h"

#include <math.h>

#ifdef DREAMCAST

#define min(x, y) ((x < y) ? (x) : (y));
#define max(x, y) ((x > y) ? (x) : (y));

extern double cos(double r);
extern double sin(double r);

#endif

#define fclamp(x, y, z) (fmin(fmax(x, y), z))

#define INF 1000000000

fup double randfrom(double tMin, double tMax);
fup int randfromInteger(int tMin, int tMax);

fup double fatan2(double y, double x);

fup double getLinearInterpolationFactor(double a, double b, double p);
fup double interpolateLinear(double a, double b, double t);

fup double fstsqrt(double x);

#endif
