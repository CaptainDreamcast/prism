#ifndef TARI_MATH
#define TARI_MATH

#define min(x, y) ((x < y) ? (x) : (y));
#define max(x, y) ((x > y) ? (x) : (y));

#define INF 1000000000

double randfrom(double tMin, double tMax);

double fatan2(double y, double x);

double getLinearInterpolationFactor(double a, double b, double p);
double interpolateLinear(double a, double b, double t);

#endif
