#include "prism/math.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

namespace prism {

	static struct {
		uint32_t mState = 1;
	} gPrismRandomData;

	void setTimeBasedRandomSeed()
	{
		setRandomSeed((unsigned int)time(NULL));
	}

	void setRandomSeed(unsigned int tSeed)
	{
		uint32_t z = tSeed + 0x9E3779B9u;
		z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
		z = (z ^ (z >> 13)) * 0xC2B2AE35u;
		z = z ^ (z >> 16);
		gPrismRandomData.mState = z ? z : 1;
	}

	// for rollback determinism tests
#if !defined(DREAMCAST) && !defined(VITA) && !defined(__EMSCRIPTEN__)
#define PRISM_COUNT_RANDOM_CALLS 1
#endif

#ifdef PRISM_COUNT_RANDOM_CALLS
	static uint64_t gPrismRandomCallAmount = 0;
#endif

	unsigned long long getRandomCallAmount()
	{
#ifdef PRISM_COUNT_RANDOM_CALLS
		return gPrismRandomCallAmount;
#else
		return 0;
#endif
	}

	static uint32_t getNextRandomInteger()
	{
#ifdef PRISM_COUNT_RANDOM_CALLS
		gPrismRandomCallAmount++;
#endif
		uint32_t x = gPrismRandomData.mState;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		gPrismRandomData.mState = x;
		return x;
	}

	unsigned int getRandomState()
	{
		return gPrismRandomData.mState;
	}

	void setRandomState(unsigned int tState)
	{
		gPrismRandomData.mState = tState ? tState : 1;
	}

	float randfrom(float tMin, float tMax) {
		const float range = (tMax - tMin);
		if (range == 0) return tMin;

		return tMin + range * (getNextRandomInteger() * (1.0f / 4294967296.0f));
	}

	int randfromInteger(int tMin, int tMax)
	{
		int val = tMin - 1;
		int iters = 0;
		while (val < tMin || val > tMax) {
			val = (int)randfrom(float(tMin), tMax + 0.99f);
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

	float fatan2(float y, float x) {
		if (x == 0.0f)
		{
			if (y > 0.0f) return PIBY2_FLOAT;
			if (y == 0.0f) return 0.0f;
			return -PIBY2_FLOAT;
		}
		float atan;
		float z = (float)(y / x);
		if (std::fabs(z) < 1.0f)
		{
			atan = z / (1.0f + 0.28f * z * z);
			if (x < 0.0f)
			{
				if (y < 0.0f) return atan - PI_FLOAT;
				return atan + PI_FLOAT;
			}
		}
		else
		{
			atan = PIBY2_FLOAT - z / (z * z + 0.28f);
			if (y < 0.0f) return atan - PI_FLOAT;
		}
		return atan;
	}

	float getLinearInterpolationFactor(float a, float b, float p) {
		return (p - a) / (b - a);
	}

	float interpolateLinear(float a, float b, float t) {
		return a + t * (b - a);
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

	Matrix4D createRotationZMatrix4D(float tAngle)
	{
		Matrix4D ret;
		ret.m[0][0] = std::cos(tAngle);
		ret.m[0][1] = std::sin(tAngle);
		ret.m[0][2] = 0;
		ret.m[0][3] = 0;

		ret.m[1][0] = -std::sin(tAngle);
		ret.m[1][1] = std::cos(tAngle);
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

	Matrix4D createOrthographicProjectionMatrix4D(float tLeft, float tRight, float tUp, float tBottom, float tNear, float tFar)
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

}