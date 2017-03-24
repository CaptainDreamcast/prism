#ifndef TARI_MATH
#define TARI_MATH

#include "common/header.h"

#define min(x, y) ((x < y) ? (x) : (y));
#define max(x, y) ((x > y) ? (x) : (y));

#define INF 1000000000

fup double randfrom(double tMin, double tMax);

fup double fatan2(double y, double x);

fup double getLinearInterpolationFactor(double a, double b, double p);
fup double interpolateLinear(double a, double b, double t);

fup double fstsqrt(double x);

#endif
