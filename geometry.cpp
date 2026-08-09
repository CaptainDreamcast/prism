#include "prism/geometry.h"

#define _USE_MATH_DEFINES  
#include <cmath> 
#include <algorithm>

#include "prism/math.h"
#include "prism/log.h"

using namespace std;
namespace prism {

Vector2D::Vector2D(float x, float y)
	: x(x)
	, y(y)
{
}

Vector3D Vector2D::xyz(float z) const
{
	return Vector3D(x, y, z);
}

Vector3D::Vector3D(float x, float y, float z)
	: x(x)
	, y(y)
	, z(z)
{
}

Vector3D::Vector3D(const Vector3DI& v)
	: x((float)v.x)
	, y ((float)v.y)
	, z ((float)v.z)
{
}

Vector2D Vector3D::xy() const
{
	return Vector2D(x, y);
}

Vector2DI::Vector2DI(int x, int y)
	: x(x)
	, y(y)
{
}

Vector3DI Vector2DI::xyz(int z) const {
	return Vector3DI(x, y, z);
}

Vector2D Vector2DI::f() const
{
	return Vector2D(float(x), float(y));
}

Vector3DI::Vector3DI(int x, int y, int z)
	: x(x)
	, y(y)
	, z(z)
{
}

Vector2DI Vector3DI::xy() const
{
	return Vector2DI(x, y);
}

Vector3D Vector3DI::f() const
{
	return Vector3D(float(x), float(y), float(z));
}

GeoRectangle2D::GeoRectangle2D(float x, float y, float w, float h)
	: mTopLeft(x, y)
	, mBottomRight(x + w, y + h)
{
}

GeoRectangle::GeoRectangle(float x, float y, float z, float w, float h)
	: mTopLeft(x, y, z)
	, mBottomRight(x + w, y + h, z)
{
}

GeoRectangle2D GeoRectangle::rect2D() const
{
	GeoRectangle2D ret;
	ret.mTopLeft = mTopLeft.xy();
	ret.mBottomRight = mBottomRight.xy();
	return ret;
}

float dot2D(const Vector2D& p1, const Vector2D& p2) {
	return p1.x*p2.x + p1.y*p2.y;
}

float dot3D(const Vector3D& p1, const Vector3D& p2) {
	return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
}

float vecLength(const Vector2D& tVelocity) {
	return fstsqrt(tVelocity.x * tVelocity.x + tVelocity.y * tVelocity.y);
}

float vecLength(const Vector2DI& tVelocity) {
	return fstsqrt(float(tVelocity.x * tVelocity.x + tVelocity.y * tVelocity.y));
}

float vecLength(const Vector3D& tVelocity) {
	return fstsqrt(tVelocity.x * tVelocity.x + tVelocity.y * tVelocity.y + tVelocity.z * tVelocity.z);
}

float vecLength(const Vector3DI& tVelocity) {
	return fstsqrt(float(tVelocity.x * tVelocity.x + tVelocity.y * tVelocity.y + tVelocity.z * tVelocity.z));
}

Vector3D vecAdd(const Vector3D& v1, const Vector3D& v2) {
	Vector3D ret;
	ret.x = v1.x + v2.x;
	ret.y = v1.y + v2.y;
	ret.z = v1.z + v2.z;
	return ret;
}

Vector3D vecSub(const Vector3D& v1, const Vector3D& v2)
{
	return vecAdd(v1, vecScale(v2, -1));
}

Vector3D vecScale(const Vector3D& v, float tFactor) {
	Vector3D ret;
	ret.x = v.x*tFactor;
	ret.y = v.y*tFactor;
	ret.z = v.z*tFactor;
	return ret;
}

Vector3D vecScale2D(const Vector3D& v, const Vector2D& tScale)
{
	return Vector3D(v.x * tScale.x, v.y * tScale.y, v.z);
}

Vector3D vecScale3D(const Vector3D& v, const Vector3D& tScale) {
	Vector3D ret;
	ret.x = v.x*tScale.x;
	ret.y = v.y*tScale.y;
	ret.z = v.z*tScale.z;
	return ret;
}

Vector2D vecNormalize(const Vector2DI& tVector) {
	float l = vecLength(tVector);
	if (l == 0) {
		return Vector2D((float)tVector.x, (float)tVector.y);
	}
	return tVector / l;
}

Vector2D vecNormalize(const Vector2D& tVector) {
	float l = vecLength(tVector);
	if (l == 0) {
		return tVector;
	}
	return tVector / l;
}

Vector3D vecNormalize(const Vector3DI& tVector) {
	float l = vecLength(tVector);
	if (l == 0) {
		return Vector3D((float)tVector.x, (float)tVector.y, (float)tVector.z);
	}
	return tVector / l;
}

Vector3D vecNormalize(const Vector3D& tVector) {
	float l = vecLength(tVector);
	if (l == 0) {
		return tVector;
	}
	return tVector / l;
}

Vector2D vecRotateZ2D(const Vector2D& tVector, float tAngle)
{
	Vector2D ret;
	ret.x = std::cos(tAngle) * tVector.x - std::sin(tAngle) * tVector.y;
	ret.y = std::sin(tAngle) * tVector.x + std::cos(tAngle) * tVector.y;
	return ret;
}

Vector3D vecRotateZ(const Vector3D& tVector, float tAngle) {
	Vector3D ret;
	ret.x = std::cos(tAngle)*tVector.x - std::sin(tAngle)*tVector.y;
	ret.y = std::sin(tAngle)*tVector.x + std::cos(tAngle)*tVector.y;
	ret.z = tVector.z;
	return ret;
}

Vector3D vecRotateZAroundCenter(const Vector3D& tVector, float tAngle, const Vector3D& tCenter) {
	auto ret = vecSub(tVector, tCenter);
	ret = vecRotateZ(ret, tAngle);
	ret = vecAdd(ret, tCenter);
	return ret;
}

Vector3D vecScaleToSize(const Vector3D& v, float tSize)
{
	return vecNormalize(v) * tSize;
}

Position variatePosition(const Position& tBase) {
	Position ret;
	ret.x = randfrom(-tBase.x, tBase.x);
	ret.y = randfrom(-tBase.y, tBase.y);
	ret.z = randfrom(-tBase.z, tBase.z);
	return ret;
}

void printPosition(char* tName, const Position& tPosition) {
	logString(tName);
	logDouble(tPosition.x);
	logDouble(tPosition.y);
	logDouble(tPosition.z);

}

Position getDirection(const Position& tFrom, const Position& tTo) {
	Position ret;
	ret.x = tTo.x - tFrom.x;
	ret.y = tTo.y - tFrom.y;
	ret.z = tTo.z - tFrom.z;
	return ret;
}

float getDistance2D(const Position& tFrom, const Position& tTo)
{
	return vecLength2D(vecSub(tTo, tFrom));
}

float getDistance2D(const Vector2D& tFrom, const Vector2D& tTo)
{
	return vecLength(tTo - tFrom);
}

Line2D makeLine2D(const Vector2D& tStart, const Vector2D& tEnd) {
	Line2D ret;
	ret.mP1 = tStart;
	ret.mP2 = tEnd;
	return ret;
}

Line makeLine(const Vector3D& tStart, const Vector3D& tEnd) {
	Line ret;
	ret.mP1 = tStart;
	ret.mP2 = tEnd;
	return ret;
}

Vector3DI vecAddI(const Vector3DI& v1, const Vector3DI& v2) {
	return v1 + v2;
}

Vector3DI vecScaleI(const Vector3DI& v, float tFactor)
{
	return Vector3DI(int(v.x * tFactor), int(v.y * tFactor), int(v.z * tFactor));
}

Vector2DI vecScaleI2D(const Vector2DI& v, float tFactor)
{
	return Vector2DI(int(v.x * tFactor), int(v.y * tFactor));
}

int vecEqualsI(const Vector3DI& v1, const Vector3DI& v2) {
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

int vecEqualsI2D(const Vector3DI& v1, const Vector3DI& v2) {
	return v1.x == v2.x && v1.y == v2.y;
}

float vecLength2D(const Vector3D& v)
{
	return vecLength(Vector3D(v.x, v.y, 0));
}

Vector3D vecAdd2D(const Vector3D& v1, const Vector3D& v2)
{
	return Vector3D(v1.x + v2.x, v1.y + v2.y, v1.z);
}

Vector3D vecSub2D(const Vector3D& v1, const Vector3D& v2)
{
	return Vector3D(v1.x - v2.x, v1.y - v2.y, v1.z);
}

Vector3D vecMin2D(const Vector3D& v1, const Vector3D& v2)
{
	return Vector3D(std::min(v1.x, v2.x), std::min(v1.y, v2.y), 0);
}

Vector3D vecMax2D(const Vector3D& v1, const Vector3D& v2)
{
	return Vector3D(std::max(v1.x, v2.x), std::max(v1.y, v2.y), 0);
}

Vector2DI vecMinI2D(const Vector2DI& v1, const Vector2DI& v2)
{
	return Vector2DI(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
}

Vector2DI vecMaxI2D(const Vector2DI& v1, const Vector2DI& v2)
{
	return Vector2DI(std::max(v1.x, v2.x), std::max(v1.y, v2.y));
}

Vector3DI vecMinI2D(const Vector3DI& v1, const Vector3DI& v2)
{
	return Vector3DI(std::min(v1.x, v2.x), std::min(v1.y, v2.y), 0);
}

Vector3DI vecMaxI2D(const Vector3DI& v1, const Vector3DI& v2)
{
	return Vector3DI(std::max(v1.x, v2.x), std::max(v1.y, v2.y), 0);
}

float getAngleFromDirection(const Vector3D& tDirection) {
	return (float)(-fatan2(tDirection.y, tDirection.x) + M_PI);
}

float getAngleFromDirection(const Vector2D& tDirection) {
	return (float)(-fatan2(tDirection.y, tDirection.x) + M_PI);
}

Vector3D getDirectionFromAngleZ(float tAngle) {
	return Vector3D(std::cos(tAngle), std::sin(tAngle), 0);
}

float degreesToRadians(float tDegrees) {
	return (float)((tDegrees / 180) * M_PI);
}

float radiansToDegrees(float tRadians) {
	return (float)((tRadians / M_PI) * 180);
}

int checkIntersectLineCircle(const Line2D& tLine, const Circle2D& tCircle) {

	float r = tCircle.mRadius;

	const auto d = tLine.mP2 - tLine.mP1;
	const auto f = tLine.mP1 - tCircle.mCenter;

	float a = dot2D(d, d);
	float b = 2 * dot2D(f, d);
	float c = dot2D(f, f) - r*r;

	float discriminant = b*b - 4 * a*c;
	if (discriminant < 0)
	{
		return 0;
	}
	else
	{
		// ray didn't totally miss sphere,
		// so there is a solution to
		// the equation.

		discriminant = fstsqrt(discriminant);

		// either solution may be on or off the ray so need to test both
		// t1 is always the smaller value, because BOTH discriminant and
		// a are nonnegative.
		float t1 = (-b - discriminant) / (2 * a);
		float t2 = (-b + discriminant) / (2 * a);

		// 3x HIT cases:
		//          -o->             --|-->  |            |  --|->
		// Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

		// 3x MISS cases:
		//       ->  o                     o ->              | -> |
		// FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

		if (t1 >= 0 && t1 <= 1)
		{
			// t1 is the intersection, and it's closer than t2
			// (since t1 uses -b - discriminant)
			// Impale, Poke
			return 1;
		}

		// here t1 didn't intersect so we are either started
		// inside the sphere or completely past it
		if (t2 >= 0 && t2 <= 1)
		{
			// ExitWound
			return 1;
		}

		// no intn: FallShort, Past, CompletelyInside
		return 0;
	}
}

int checkPointInCircle(const Circle2D& tCirc, const Position2D& tPoint) {
	const auto d = tPoint - tCirc.mCenter;
	float l = vecLength(d);
	return l <= tCirc.mRadius;
}

int checkPointInRectangle(const GeoRectangle2D& tRect, const Position2D& tPoint) {
	return tPoint.x >= tRect.mTopLeft.x && tPoint.x <= tRect.mBottomRight.x && tPoint.y >= tRect.mTopLeft.y && tPoint.y <= tRect.mBottomRight.y;
}

int checkIntersectCircRect(const Circle2D& tCirc, const GeoRectangle2D& tRect) {
	const auto p1 = tRect.mTopLeft;
	const auto p2 = Vector2D(tRect.mTopLeft.x, tRect.mBottomRight.y);
	const auto p3 = tRect.mBottomRight;
	const auto p4 = Vector2D(tRect.mBottomRight.x, tRect.mTopLeft.y);

	return (checkPointInRectangle(tRect, tCirc.mCenter) ||
		checkIntersectLineCircle(makeLine2D(p1, p2), tCirc) ||
		checkIntersectLineCircle(makeLine2D(p2, p3), tCirc) ||
		checkIntersectLineCircle(makeLine2D(p3, p4), tCirc) ||
		checkIntersectLineCircle(makeLine2D(p4, p1), tCirc) ||
		(checkPointInCircle(tCirc, p1) && 
			checkPointInCircle(tCirc, p2) && 
			checkPointInCircle(tCirc, p3)  && 
			checkPointInCircle(tCirc, p4)));

}

Vector2D clampPositionToGeoRectangle(const Vector2D& v, const GeoRectangle2D & tRect)
{
	Vector2D ret = v;
	ret.x = std::fmax(ret.x, tRect.mTopLeft.x);
	ret.y = std::fmax(ret.y, tRect.mTopLeft.y);
	ret.x = std::fmin(ret.x, tRect.mBottomRight.x);
	ret.y = std::fmin(ret.y, tRect.mBottomRight.y);
	return ret;
}

Vector2DI clampPositionToGeoRectangle(const Vector2DI& v, const GeoRectangle2D& tRect)
{
	Vector2DI ret = v;
	ret.x = max(ret.x, int(tRect.mTopLeft.x));
	ret.y = max(ret.y, int(tRect.mTopLeft.y));
	ret.x = min(ret.x, int(tRect.mBottomRight.x));
	ret.y = min(ret.y, int(tRect.mBottomRight.y));
	return ret;
}

Vector3D clampPositionToGeoRectangle(const Vector3D & v, const GeoRectangle2D & tRect)
{
	Vector3D ret = v;
	ret.x = std::fmax(ret.x, tRect.mTopLeft.x);
	ret.y = std::fmax(ret.y, tRect.mTopLeft.y);
	ret.x = std::fmin(ret.x, tRect.mBottomRight.x);
	ret.y = std::fmin(ret.y, tRect.mBottomRight.y);
	return ret;
}

Vector3DI clampPositionToGeoRectangle(const Vector3DI& v, const GeoRectangle2D& tRect)
{
	Vector3DI ret = v;
	ret.x = max(ret.x, int(tRect.mTopLeft.x));
	ret.y = max(ret.y, int(tRect.mTopLeft.y));
	ret.x = min(ret.x, int(tRect.mBottomRight.x));
	ret.y = min(ret.y, int(tRect.mBottomRight.y));
	return ret;
}

Vector3DI clampPositionToGeoRectangle(const Vector3DI& v, const GeoRectangle& tRect)
{
	Vector3DI ret = v;
	ret.x = max(ret.x, int(tRect.mTopLeft.x));
	ret.y = max(ret.y, int(tRect.mTopLeft.y));
	ret.x = min(ret.x, int(tRect.mBottomRight.x));
	ret.y = min(ret.y, int(tRect.mBottomRight.y));
	return ret;
}

Vector3D clampPositionToGeoRectangle(const Vector3D& v, const GeoRectangle& tRect)
{
	Vector3D ret = v;
	ret.x = std::fmax(ret.x, tRect.mTopLeft.x);
	ret.y = std::fmax(ret.y, tRect.mTopLeft.y);
	ret.x = std::fmin(ret.x, tRect.mBottomRight.x);
	ret.y = std::fmin(ret.y, tRect.mBottomRight.y);
	return ret;
}

GeoRectangle2D scaleGeoRectangleByFactor(const GeoRectangle2D& tRect, float tFac)
{
	return scaleGeoRectangleByFactor2D(tRect, Vector2D(tFac, tFac));
}

GeoRectangle scaleGeoRectangleByFactor(const GeoRectangle& tRect, float tFac)
{
	return scaleGeoRectangleByFactor2D(tRect, Vector2D(tFac, tFac));
}

GeoRectangle2D scaleGeoRectangleByFactor2D(const GeoRectangle2D& tRect, const Vector2D& tFac) {
	GeoRectangle2D ret = tRect;
	ret.mTopLeft.x *= tFac.x;
	ret.mTopLeft.y *= tFac.y;
	ret.mBottomRight.x *= tFac.x;
	ret.mBottomRight.y *= tFac.y;
	return ret;
}

GeoRectangle2D scaleGeoRectangleByFactor2D(const GeoRectangle2D& tRect, const Vector3D& tFac) 
{
	GeoRectangle2D ret = tRect;
	ret.mTopLeft.x *= tFac.x;
	ret.mTopLeft.y *= tFac.y;
	ret.mBottomRight.x *= tFac.x;
	ret.mBottomRight.y *= tFac.y;
	return ret;
}

GeoRectangle scaleGeoRectangleByFactor2D(const GeoRectangle & tRect, const Vector2D& tFac)
{
	GeoRectangle ret = tRect;
	ret.mTopLeft.x *= tFac.x;
	ret.mTopLeft.y *= tFac.y;
	ret.mBottomRight.x *= tFac.x;
	ret.mBottomRight.y *= tFac.y;
	return ret;
}

GeoRectangle scaleGeoRectangleByFactor2D(const GeoRectangle& tRect, const Vector3D& tFac)
{
	GeoRectangle ret = tRect;
	ret.mTopLeft.x *= tFac.x;
	ret.mTopLeft.y *= tFac.y;
	ret.mBottomRight.x *= tFac.x;
	ret.mBottomRight.y *= tFac.y;
	return ret;
}

Vector3D interpolatePositionLinear(const Position& a, const Position& b, float t)
{
	Vector3D ret;
	ret.x = interpolateLinear(a.x, b.x, t);
	ret.y = interpolateLinear(a.y, b.y, t);
	ret.z = interpolateLinear(a.z, b.z, t);
	return ret;
}

Vector2D operator+(const Vector2D& a, const Vector2D& b)
{
	return Vector2D(a.x + b.x, a.y + b.y);
}

Vector2D operator+(const Vector2D& a, const Vector2DI& b)
{
	return Vector2D(a.x + b.x, a.y + b.y);
}

Vector3D operator+(const Vector2D& a, const Vector3D& b)
{
	return Vector3D(a.x + b.x, a.y + b.y, b.z);
}

Vector3D operator+(const Vector2D& a, const Vector3DI& b)
{
	return Vector3D(a.x + b.x, a.y + b.y, float(b.z));
}

GeoRectangle2D operator+(const Vector2D& a, const GeoRectangle2D& b)
{
	GeoRectangle2D ret = b;
	ret.mTopLeft = ret.mTopLeft + a;
	ret.mBottomRight = ret.mBottomRight + a;
	return ret;
}

Vector2D operator-(const Vector2D& a, const Vector2D& b)
{
	return Vector2D(a.x - b.x, a.y - b.y);
}

Vector2D operator-(const Vector2D& a, const Vector2DI& b)
{
	return Vector2D(a.x - b.x, a.y - b.y);
}

Vector3D operator-(const Vector2D& a, const Vector3D& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, b.z);
}
Vector3D operator-(const Vector2D& a, const Vector3DI& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, float(b.z));
}
GeoRectangle2D operator-(const Vector2D& a, const GeoRectangle2D& b)
{
	GeoRectangle2D ret = b;
	ret.mTopLeft = a - ret.mTopLeft;
	ret.mBottomRight = a - ret.mBottomRight;
	return ret;
}

Vector3D operator+(const Vector3D & a, const Vector2D& b)
{
	return Vector3D(a.x + b.x, a.y + b.y, a.z);
}

Vector3D operator+(const Vector3D & a, const Vector2DI& b)
{
	return Vector3D(a.x + b.x, a.y + b.y, a.z);
}

Vector3D operator-(const Vector3D & a, const Vector2D& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, a.z);
}

Vector3D operator-(const Vector3D& a, const Vector2DI& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, a.z);
}

Vector3D operator-(const Vector3D& a, const Vector3DI& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector2D operator*(const float & a, const Vector2D& b)
{
	return Vector2D(b.x * a, b.y * a);
}

Vector2D operator*(const Vector2D& a, const float & b)
{
	return Vector2D(a.x * b, a.y * b);
}

Vector2D operator*(const Vector2D& a, const Vector2D& b)
{
	return Vector2D(a.x * b.x, a.y * b.y);
}

Vector2D operator+(const Vector2DI& a, const Vector2D& b)
{
	return Vector2D(a.x + b.x, a.y + b.y);
}

Vector3D operator+(const Vector2DI& a, const Vector3D& b)
{
	return Vector3D(a.x + b.x, a.y + b.y, b.z);
}

Vector3DI operator+(const Vector2DI& a, const Vector3DI& b)
{
	return Vector3DI(a.x + b.x, a.y + b.y, b.z);
}

Vector2D operator-(const Vector2DI& a, const Vector2D& b)
{
	return Vector2D(a.x - b.x, a.y - b.y);
}

Vector3D operator-(const Vector2DI& a, const Vector3D& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, b.z);
}

Vector3DI operator-(const Vector2DI& a, const Vector3DI& b)
{
	return Vector3DI(a.x - b.x, a.y - b.y, b.z);
}

Vector2DI operator*(const Vector2DI& a, const int& b)
{
	return Vector2DI(a.x * b, a.y * b);
}

Vector2DI operator*(const Vector2DI& a, const Vector2DI& b)
{
	return Vector2DI(a.x * b.x, a.y * b.y);
}

Vector2DI operator/(const Vector2DI& a, const int& b)
{
	return Vector2DI(a.x / b, a.y / b);
}

int operator!=(const Vector2DI& a, const Vector2DI& b)
{
	return a.x != b.x || a.y != b.y;
}

Vector2D operator*(const float& a, const Vector2DI& b)
{
	return Vector2D(b.x * a, b.y * a);
}

Vector3DI operator*(const float& a, const Vector3DI& b)
{
	return Vector3DI((int)(b.x * a), (int)(b.y * a), (int)(b.z * a));
}

Vector2D operator/(const float& a, const Vector2DI& b)
{
	return Vector2D(a / b.x, a / b.y);
}

Vector3D operator/(const float& a, const Vector3DI& b)
{
	return Vector3D(a / b.x, a / b.y, a / b.z);
}

Vector2DI operator*(const int& a, const Vector2DI& b)
{
	return Vector2DI(b.x * a, b.y * a);
}

Vector3DI operator*(const int& a, const Vector3DI& b)
{
	return Vector3DI(b.x * a, b.y * a, b.z * a);
}

Vector3D operator*(const Vector3D& a, const Vector2D& b)
{
	return Vector3D(a.x * b.x, a.y * b.y, a.z);
}

Vector3DI operator*(const Vector3DI& a, const int& b)
{
	return Vector3DI(a.x * b, a.y * b, a.z * b);
}
Vector3D operator*(const Vector3DI& a, const float& b)
{
	return Vector3D(a.x * b, a.y * b, a.z * b);
}

Vector3D operator+(const Vector3DI& a, const Vector2D& b)
{
	return Vector3D(a.x + b.x, a.y + b.y, float(a.z));
}

Vector3DI operator+(const Vector3DI& a, const Vector2DI& b)
{
	return Vector3DI(a.x + b.x, a.y + b.y, a.z);
}

Vector3D operator-(const Vector3DI& a, const Vector2D& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, float(a.z));
}

Vector3DI operator-(const Vector3DI& a, const Vector2DI& b)
{
	return Vector3DI(a.x - b.x, a.y - b.y, a.z);
}

Vector3D operator-(const Vector3DI& a, const Vector3D& b)
{
	return Vector3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector2D operator/(const Vector2D& a, const float & b)
{
	return Vector2D(a.x / b, a.y / b);
}

Vector2D operator/(const float & a, const Vector2D& b)
{
	return Vector2D(a / b.x, a / b.y);
}

int operator==(const Vector2D& a, const Vector2D& b) {
	return a.x == b.x && a.y == b.y;
}

int operator!=(const Vector2D& a, const Vector2D& b) {
	return a.x != b.x || a.y != b.y;
}

Vector3D operator+(const Vector3D& a, const Vector3D& b) {
	return Vector3D(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3D operator-(const Vector3D& a, const Vector3D& b) {
	return Vector3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3D operator*(const float & a, const Vector3D & b) {
	return Vector3D(b.x * a, b.y * a, b.z * a);
}

Vector3D operator*(const Vector3D& a, const float& b) {
	return Vector3D(a.x * b, a.y * b, a.z * b);
}

Vector3D operator/(const Vector3D& a, const float& b) {
	return Vector3D(a.x / b, a.y / b, a.z / b);
}

Vector3D operator/(const float& a, const Vector3D& b) {
	return Vector3D(a / b.x, a / b.y, a / b.z);
}

Vector2DI operator+(const Vector2DI& a, const Vector2DI& b) {
	return Vector2DI(a.x + b.x, a.y + b.y);
}

Vector3DI operator+(const Vector3DI& a, const Vector3DI& b) {
	return Vector3DI(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3D operator+(const Vector3DI& a, const Vector3D& b) {
	return Vector3D(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3D operator+(const Vector3D& a, const Vector3DI& b) {
	return Vector3D(a.x + b.x, a.y + b.y, a.z + b.z);
}

GeoRectangle2D operator+(const Vector3D& a, const GeoRectangle2D& b)
{
	return b + a.xy();
}

Vector3D operator*(const Vector3D& a, const Vector3D& b) {
	Vector3D ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}

int operator==(const Vector3D& a, const Vector3D& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

int operator!=(const Vector3D& a, const Vector3D& b) {
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

Vector3D& operator+=(Vector3D& a, const Vector2D& b)
{
	a = a + b;
	return a;
}

Vector3D& operator+=(Vector3D& a, const Vector3D& b) {
	a = a + b;
	return a;
}

Vector3D & operator-=(Vector3D & a, const Vector2D& b)
{
	a = a - b;
	return a;
}

Vector2DI operator-(const Vector2DI& a, const Vector2DI& b) {
	return Vector2DI(a.x - b.x, a.y - b.y);
}

Vector3DI operator-(const Vector3DI& a, const Vector3DI& b) {
	Vector3DI ret = a;
	ret.x -= b.x;
	ret.y -= b.y;
	ret.z -= b.z;
	return ret;
}

Vector2D operator*(const Vector2DI & a, const float & b)
{
	return Vector2D(a.x * b, a.y * b);
}

Vector2D operator/(const Vector2DI& a, const float& b) {
	return Vector2D(a.x / b, a.y / b);
}

Vector3D operator/(const Vector3DI& a, const float& b) {
	return Vector3D(a.x / b, a.y / b, a.z / b);
}

Vector3DI operator/(const Vector3DI& a, const int& b) {
	return Vector3DI(a.x / b, a.y / b, a.z / b);
}

Vector3DI operator*(const Vector3DI& a, const Vector3DI& b) {
	Vector3DI ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}

int operator==(const Vector2DI& a, const Vector2DI& b) {
	return a.x == b.x && a.y == b.y;
}

int operator==(const Vector3DI& a, const Vector3DI& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

int operator!=(const Vector3DI& a, const Vector3DI& b) {
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

GeoRectangle2D operator*(const GeoRectangle2D & a, const float & b)
{
	GeoRectangle2D ret;
	ret.mTopLeft = a.mTopLeft*b;
	ret.mBottomRight = a.mBottomRight*b;
	return ret;
}

GeoRectangle2D operator+(const GeoRectangle2D & a, const Position2D & b)
{
	GeoRectangle2D ret;
	ret.mTopLeft = a.mTopLeft + b;
	ret.mBottomRight = a.mBottomRight + b;
	return ret;
}

GeoRectangle operator*(const GeoRectangle& a, const float& b) {
	GeoRectangle ret;
	ret.mTopLeft = a.mTopLeft*b;
	ret.mBottomRight = a.mBottomRight*b;
	return ret;
}

GeoRectangle operator+(const GeoRectangle& a, const Position& b) {
	GeoRectangle ret;
	ret.mTopLeft = a.mTopLeft + b;
	ret.mBottomRight = a.mBottomRight + b;
	return ret;
}

GeoRectangle2D operator-(const GeoRectangle2D& a, const Position2D& b)
{
	GeoRectangle2D ret;
	ret.mTopLeft = a.mTopLeft - b;
	ret.mBottomRight = a.mBottomRight - b;
	return ret;
}

GeoRectangle2D operator/(const GeoRectangle2D& a, const float& b)
{
	GeoRectangle2D ret;
	ret.mTopLeft = a.mTopLeft / b;
	ret.mBottomRight = a.mBottomRight / b;
	return ret;
}

Vector2D& operator+=(Vector2D& a, const Vector2D& b)
{
	a = a + b;
	return a;
}
Vector2D& operator+=(Vector2D& a, const Vector2DI& b)
{
	a = a + b;
	return a;
}
Vector2D& operator-=(Vector2D& a, const Vector2D& b)
{
	a = a - b;
	return a;
}
Vector2D& operator-=(Vector2D& a, const Vector2DI& b)
{
	a = a - b;
	return a;
}
Vector2D& operator*=(Vector2D& a, const float& b)
{
	a = a * b;
	return a;
}
Vector2D& operator*=(Vector2D& a, const Vector2D& b)
{
	a = a * b;
	return a;
}
Vector2D& operator/=(Vector2D& a, const float& b)
{
	a = a / b;
	return a;
}

Vector2DI& operator+=(Vector2DI& a, const Vector2DI& b)
{
	a = a + b;
	return a;
}
Vector2DI& operator-=(Vector2DI& a, const Vector2DI& b)
{
	a = a - b;
	return a;
}
Vector2DI& operator*=(Vector2DI& a, const int& b)
{
	a = a * b;
	return a;
}
Vector2DI& operator*=(Vector2DI& a, const Vector2DI& b)
{
	a = a * b;
	return a;
}
Vector2DI& operator/=(Vector2DI& a, const int& b)
{
	a = a / b;
	return a;
}

Vector3D& operator+=(Vector3D& a, const Vector2DI& b)
{
	a = a + b;
	return a;
}

Vector3D& operator+=(Vector3D& a, const Vector3DI& b)
{
	a = a + b;
	return a;
}

Vector3D& operator-=(Vector3D& a, const Vector2DI& b)
{
	a = a - b;
	return a;
}
Vector3D& operator-=(Vector3D& a, const Vector3D& b)
{
	a = a - b;
	return a;
}
Vector3D& operator-=(Vector3D& a, const Vector3DI& b)
{
	a = a - b;
	return a;
}
Vector3D& operator*=(Vector3D& a, const float& b)
{
	a = a * b;
	return a;
}
Vector3D& operator*=(Vector3D& a, const Vector3D& b)
{
	a = a * b;
	return a;
}
Vector3D& operator*=(Vector3D& a, const Vector2D& b)
{
	a = a * b;
	return a;
}
Vector3D& operator/=(Vector3D& a, const float& b)
{
	a = a / b;
	return a;
}

Vector3DI& operator+=(Vector3DI& a, const Vector2DI& b)
{
	a = a + b;
	return a;
}
Vector3DI& operator+=(Vector3DI& a, const Vector3DI& b)
{
	a = a + b;
	return a;
}
Vector3DI& operator-=(Vector3DI& a, const Vector2DI& b)
{
	a = a - b;
	return a;
}
Vector3DI& operator-=(Vector3DI& a, const Vector3DI& b)
{
	a = a - b;
	return a;
}
Vector3DI& operator*=(Vector3DI& a, const int& b)
{
	a = a * b;
	return a;
}
Vector3DI& operator*=(Vector3DI& a, const Vector3DI& b)
{
	a = a * b;
	return a;
}
Vector3DI& operator/=(Vector3DI& a, const int& b)
{
	a = a / b;
	return a;
}

GeoRectangle2D& operator+=(GeoRectangle2D& a, const Position2D& b)
{
	a = a + b;
	return a;
}
GeoRectangle2D& operator-=(GeoRectangle2D& a, const Position2D& b)
{
	a = a - b;
	return a;
}
GeoRectangle2D& operator*=(GeoRectangle2D& a, const float& b)
{
	a = a * b;
	return a;
}
GeoRectangle2D& operator/=(GeoRectangle2D& a, const float& b)
{
	a = a / b;
	return a;
}

}