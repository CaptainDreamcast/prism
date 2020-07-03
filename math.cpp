#include "prism/math.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

void setTimeBasedRandomSeed()
{
	setRandomSeed((unsigned int)time(NULL));
}

void setRandomSeed(unsigned int tSeed)
{
	srand(tSeed);
}

double randfrom(double tMin, double tMax) {
	double range = (tMax - tMin); 
	if(range == 0) return tMin;

	double div = RAND_MAX / range;
	return tMin + (rand() / div);
}

int randfromInteger(int tMin, int tMax)
{
	int val = tMin - 1;
	int iters = 0;
	while (val < tMin || val > tMax) {
		val = (int)randfrom(tMin, tMax + 0.99);
		if (iters++ > 100) break;
	}
	return val;
}

Position randPositionInGeoRectangle(const GeoRectangle& tRectangle)
{
	return Vector3D(randfrom(tRectangle.mTopLeft.x, tRectangle.mBottomRight.x), randfrom(tRectangle.mTopLeft.y, tRectangle.mBottomRight.y), randfrom(tRectangle.mTopLeft.z, tRectangle.mBottomRight.z));
}

#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f

double fatan2(double y, double x){
	if ( x == 0.0f )
	{
		if ( y > 0.0f ) return PIBY2_FLOAT;
		if ( y == 0.0f ) return 0.0f;
		return -PIBY2_FLOAT;
	}
	float atan;
	float z = (float)(y/x);
	if ( fabs( z ) < 1.0f )
	{
		atan = z/(1.0f + 0.28f*z*z);
		if ( x < 0.0f )
		{
			if ( y < 0.0f ) return atan - PI_FLOAT;
			return atan + PI_FLOAT;
		}
	}
	else
	{
		atan = PIBY2_FLOAT - z/(z*z + 0.28f);
		if ( y < 0.0f ) return atan - PI_FLOAT;
	}
	return atan;
}

double getLinearInterpolationFactor(double a, double b, double p) {
	return (p - a) / (b - a);
}

double interpolateLinear(double a, double b, double t) {
	return a + t * (b-a);
}

Matrix4D makeIdentityMatrix4D()
{
	Matrix4D ret;
	ret.m[0][0] = 1;
	ret.m[0][1] = 0;
	ret.m[0][2] = 0;
	ret.m[0][3] = 0;

	ret.m[1][0] = 0;
	ret.m[1][1] = 1;
	ret.m[1][2] = 0;
	ret.m[1][3] = 0;

	ret.m[2][0] = 0;
	ret.m[2][1] = 0;
	ret.m[2][2] = 1;
	ret.m[2][3] = 0;

	ret.m[3][0] = 0;
	ret.m[3][1] = 0;
	ret.m[3][2] = 0;
	ret.m[3][3] = 1;

	return ret;
}

Matrix4D matMult4D(const Matrix4D& tA, const Matrix4D& tB)
{
	Matrix4D ret;
	
	int i, j, k;
	for (j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) {
			ret.m[j][i] = 0;
			for (k = 0; k < 4; k++) {
				ret.m[j][i] += tA.m[k][i] * tB.m[j][k];
			}
		}
	}
	
	return ret;
}

Matrix4D createScaleMatrix4D(const Vector2D& tScale)
{
	return createScaleMatrix4D(tScale.xyz(1.0));
}

Matrix4D createScaleMatrix4D(const Vector3D& tScale)
{
	Matrix4D ret;
	ret.m[0][0] = tScale.x;
	ret.m[0][1] = 0;
	ret.m[0][2] = 0;
	ret.m[0][3] = 0;

	ret.m[1][0] = 0;
	ret.m[1][1] = tScale.y;
	ret.m[1][2] = 0;
	ret.m[1][3] = 0;

	ret.m[2][0] = 0;
	ret.m[2][1] = 0;
	ret.m[2][2] = tScale.z;
	ret.m[2][3] = 0;

	ret.m[3][0] = 0;
	ret.m[3][1] = 0;
	ret.m[3][2] = 0;
	ret.m[3][3] = 1;

	return ret;
}

Matrix4D createTranslationMatrix4D(const Vector2D& tTranslation)
{
	return createTranslationMatrix4D(tTranslation.xyz(0.0));
}

Matrix4D createTranslationMatrix4D(const Vector3D& tTranslation)
{
	Matrix4D ret;
	ret.m[0][0] = 1;
	ret.m[0][1] = 0;
	ret.m[0][2] = 0;
	ret.m[0][3] = 0;

	ret.m[1][0] = 0;
	ret.m[1][1] = 1;
	ret.m[1][2] = 0;
	ret.m[1][3] = 0;

	ret.m[2][0] = 0;
	ret.m[2][1] = 0;
	ret.m[2][2] = 1;
	ret.m[2][3] = 0;

	ret.m[3][0] = tTranslation.x;
	ret.m[3][1] = tTranslation.y;
	ret.m[3][2] = tTranslation.z;
	ret.m[3][3] = 1;

	return ret;
}

Matrix4D createRotationZMatrix4D(double tAngle)
{
	Matrix4D ret;
	ret.m[0][0] = cos(tAngle);
	ret.m[0][1] = sin(tAngle);
	ret.m[0][2] = 0;
	ret.m[0][3] = 0;

	ret.m[1][0] = -sin(tAngle);
	ret.m[1][1] = cos(tAngle);
	ret.m[1][2] = 0;
	ret.m[1][3] = 0;

	ret.m[2][0] = 0;
	ret.m[2][1] = 0;
	ret.m[2][2] = 1;
	ret.m[2][3] = 0;

	ret.m[3][0] = 0;
	ret.m[3][1] = 0;
	ret.m[3][2] = 0;
	ret.m[3][3] = 1;

	return ret;
}

Matrix4D createOrthographicProjectionMatrix4D(double tLeft, double tRight, double tUp, double tBottom, double tNear, double tFar)
{
	Matrix4D ret;
	ret.m[0][0] = 2 / (tRight - tLeft);
	ret.m[0][1] = 0;
	ret.m[0][2] = 0;
	ret.m[0][3] = 0;

	ret.m[1][0] = 0;
	ret.m[1][1] = 2 / (tUp - tBottom);
	ret.m[1][2] = 0;
	ret.m[1][3] = 0;

	ret.m[2][0] = 0;
	ret.m[2][1] = 0;
	ret.m[2][2] = -2 / (tFar - tNear);
	ret.m[2][3] = 0;

	ret.m[3][0] = -((tRight + tLeft) / (tRight - tLeft));
	ret.m[3][1] = -((tUp + tBottom) / (tUp - tBottom));
	ret.m[3][2] = -((tFar + tNear) / (tFar - tNear));
	ret.m[3][3] = 1;

	return ret;
}

Position rotateScaleTranslatePositionByMatrix4D(const Matrix4D& tMatrix, const Position& tPosition)
{
	Position ret;
	ret.x = tMatrix.m[0][0] * tPosition.x + tMatrix.m[1][0] * tPosition.y + tMatrix.m[2][0] * tPosition.z + tMatrix.m[3][0];
	ret.y = tMatrix.m[0][1] * tPosition.x + tMatrix.m[1][1] * tPosition.y + tMatrix.m[2][1] * tPosition.z + tMatrix.m[3][1];
	ret.z = tMatrix.m[0][2] * tPosition.x + tMatrix.m[1][2] * tPosition.y + tMatrix.m[2][2] * tPosition.z + tMatrix.m[3][2];
	return ret;
}
