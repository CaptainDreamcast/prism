#pragma once

namespace prism {

struct Vector3D;
struct Vector3DI;

struct Vector2D {
	inline Vector2D() {};
	Vector2D(float x, float y);
	Vector3D xyz(float z) const;
	float x;
	float y;
};
using Position2D = Vector2D;

struct Vector3D {
	inline Vector3D() {};
	Vector3D(float x, float y, float z);
	Vector3D(const Vector3DI& v);
	Vector2D xy() const;
	float x;
	float y;
	float z;
};
using Position = Vector3D;

struct Vector2DI {
	inline Vector2DI() {};
	Vector2DI(int x, int y);
	Vector3DI xyz(int z) const;
	Vector2D f() const;
	int x;
	int y;
};

struct Vector3DI {
	inline Vector3DI() {};
	Vector3DI(int x, int y, int z);
	Vector2DI xy() const;
	Vector3D f() const;
	int x;
	int y;
	int z;
};

struct GeoRectangle2D {
	inline GeoRectangle2D() {};
	GeoRectangle2D(float x, float y, float w, float h);
	Position2D mTopLeft;
	Position2D mBottomRight;
};

struct GeoRectangle {
	inline GeoRectangle() {};
	GeoRectangle(float x, float y, float z, float w, float h);
	GeoRectangle2D rect2D() const;
	Position mTopLeft;
	Position mBottomRight;
};

typedef struct {
  Position2D mCenter;
  float mRadius;
} Circle2D;

typedef struct {
	Position mCenter;
	float mRadius;
} Circle;

typedef struct {
	Position2D mP1;
	Position2D mP2;
} Line2D;

typedef struct{
	Position mP1;
	Position mP2;
} Line;

float dot2D(const Vector2D& p1, const Vector2D& p2);
float dot3D(const Vector3D& p1, const Vector3D& p2);

Position variatePosition(const Position& tBase);
void printPosition(char* tName, const Position& tPosition);

float vecLength(const Vector2D& v);
float vecLength(const Vector2DI& v);
float vecLength(const Vector3D& v);
float vecLength(const Vector3DI& v);
Vector3D vecAdd(const Vector3D& v1, const Vector3D& v2);
Vector3D vecSub(const Vector3D& v1, const Vector3D& v2);
Vector3D vecScale(const Vector3D& v, float tFactor);
Vector3D vecScale2D(const Vector3D& v, const Vector2D& tScale);
Vector3D vecScale3D(const Vector3D& v, const Vector3D& tScale);
Vector2D vecNormalize(const Vector2DI& tVector);
Vector2D vecNormalize(const Vector2D& tVector);
Vector3D vecNormalize(const Vector3DI& tVector);
Vector3D vecNormalize(const Vector3D& tVector);
Vector2D vecRotateZ2D(const Vector2D& tVector, float tAngle);
Vector3D vecRotateZ(const Vector3D& tVector, float tAngle);
Vector3D vecRotateZAroundCenter(const Vector3D& tVector, float tAngle, const Vector3D& tCenter);
Vector3D vecScaleToSize(const Vector3D& v, float tSize);
Position getDirection(const Position& tFrom, const Position& tTo);
float getDistance2D(const Position& tFrom, const Position& tTo);
float getDistance2D(const Vector2D& tFrom, const Vector2D& tTo);
Line2D makeLine2D(const Vector2D& tStart, const Vector2D& tEnd);
Line makeLine(const Vector3D& tStart, const Vector3D& tEnd);

Vector3DI vecAddI(const Vector3DI& v1, const Vector3DI& v2);
Vector3DI vecScaleI(const Vector3DI& v, float tFactor);
Vector2DI vecScaleI2D(const Vector2DI& v, float tFactor);
int vecEqualsI(const Vector3DI& v1, const Vector3DI& v2);
int vecEqualsI2D(const Vector3DI& v1, const Vector3DI& v2);

float vecLength2D(const Vector3D& v);
Vector3D vecAdd2D(const Vector3D& v1, const Vector3D& v2);
Vector3D vecSub2D(const Vector3D& v1, const Vector3D& v2);
Vector3D vecMin2D(const Vector3D& v1, const Vector3D& v2);
Vector3D vecMax2D(const Vector3D& v1, const Vector3D& v2);

Vector2DI vecMinI2D(const Vector2DI& v1, const Vector2DI& v2);
Vector2DI vecMaxI2D(const Vector2DI& v1, const Vector2DI& v2);
Vector3DI vecMinI2D(const Vector3DI& v1, const Vector3DI& v2);
Vector3DI vecMaxI2D(const Vector3DI& v1, const Vector3DI& v2);

float getAngleFromDirection(const Vector3D& tDirection);
float getAngleFromDirection(const Vector2D& tDirection);
Vector3D getDirectionFromAngleZ(float tAngle);
float degreesToRadians(float tDegrees);
float radiansToDegrees(float tRadians);

int checkIntersectLineCircle(const Line2D& tLine, const Circle2D& tCircle);

int checkPointInCircle(const Circle2D& tCirc, const Position2D& tPoint);
int checkPointInRectangle(const GeoRectangle2D& tRect, const Position2D& tPoint);
int checkIntersectCircRect(const Circle2D& tCirc, const GeoRectangle2D& tRect);

Vector2D clampPositionToGeoRectangle(const Vector2D& v, const GeoRectangle2D& tRect);
Vector2DI clampPositionToGeoRectangle(const Vector2DI& v, const GeoRectangle2D& tRect);
Vector3D clampPositionToGeoRectangle(const Vector3D& v, const GeoRectangle2D& tRect);
Vector3D clampPositionToGeoRectangle(const Vector3D& v, const GeoRectangle& tRect);
Vector3DI clampPositionToGeoRectangle(const Vector3DI& v, const GeoRectangle2D& tRect);
Vector3DI clampPositionToGeoRectangle(const Vector3DI& v, const GeoRectangle& tRect);
GeoRectangle2D scaleGeoRectangleByFactor(const GeoRectangle2D& tRect, float tFac);
GeoRectangle scaleGeoRectangleByFactor(const GeoRectangle& tRect, float tFac);
GeoRectangle2D scaleGeoRectangleByFactor2D(const GeoRectangle2D& tRect, const Vector2D& tFac);
GeoRectangle2D scaleGeoRectangleByFactor2D(const GeoRectangle2D& tRect, const Vector3D& tFac);
GeoRectangle scaleGeoRectangleByFactor2D(const GeoRectangle& tRect, const Vector2D& tFac);
GeoRectangle scaleGeoRectangleByFactor2D(const GeoRectangle& tRect, const Vector3D& tFac);

Vector3D interpolatePositionLinear(const Position& a, const Position& b, float t);

Vector2D operator+(const Vector2D& a, const Vector2D& b);
Vector2D operator+(const Vector2D& a, const Vector2DI& b);
Vector3D operator+(const Vector2D& a, const Vector3D& b);
Vector3D operator+(const Vector2D& a, const Vector3DI& b);
GeoRectangle2D operator+(const Vector2D& a, const GeoRectangle2D& b);
Vector2D operator-(const Vector2D& a, const Vector2D& b);
Vector2D operator-(const Vector2D& a, const Vector2DI& b);
Vector3D operator-(const Vector2D& a, const Vector3D& b);
Vector3D operator-(const Vector2D& a, const Vector3DI& b);
Vector2D operator*(const Vector2D& a, const float& b);
// pairwise multiplication
Vector2D operator*(const Vector2D& a, const Vector2D& b);
Vector2D operator/(const Vector2D& a, const float& b);
int operator==(const Vector2D& a, const Vector2D& b);
int operator!=(const Vector2D& a, const Vector2D& b);

Vector2D operator+(const Vector2DI& a, const Vector2D& b);
Vector2DI operator+(const Vector2DI& a, const Vector2DI& b);
Vector3D operator+(const Vector2DI& a, const Vector3D& b);
Vector3DI operator+(const Vector2DI& a, const Vector3DI& b);
Vector2D operator-(const Vector2DI& a, const Vector2D& b);
Vector2DI operator-(const Vector2DI& a, const Vector2DI& b);
Vector3D operator-(const Vector2DI& a, const Vector3D& b);
Vector3DI operator-(const Vector2DI& a, const Vector3DI& b);
Vector2DI operator*(const Vector2DI& a, const int& b);
Vector2D operator*(const Vector2DI& a, const float& b);
// pairwise multiplication
Vector2DI operator*(const Vector2DI& a, const Vector2DI& b);
Vector2DI operator/(const Vector2DI& a, const int& b);
Vector2D operator/(const Vector2DI& a, const float& b);
int operator==(const Vector2DI& a, const Vector2DI& b);
int operator!=(const Vector2DI& a, const Vector2DI& b);

Vector3D operator+(const Vector3D& a, const Vector2D& b);
Vector3D operator+(const Vector3D& a, const Vector2DI& b);
Vector3D operator+(const Vector3D& a, const Vector3D& b);
Vector3D operator+(const Vector3D& a, const Vector3DI& b);
GeoRectangle2D operator+(const Vector3D& a, const GeoRectangle2D& b);
Vector3D operator-(const Vector3D& a, const Vector2D& b);
Vector3D operator-(const Vector3D& a, const Vector2DI& b);
Vector3D operator-(const Vector3D& a, const Vector3D& b);
Vector3D operator-(const Vector3D& a, const Vector3DI& b);
Vector3D operator*(const Vector3D& a, const float& b);
// pairwise multiplication
Vector3D operator*(const Vector3D& a, const Vector3D& b); 
Vector3D operator*(const Vector3D& a, const Vector2D& b);
Vector3D operator/(const Vector3D& a, const float& b);
int operator==(const Vector3D& a, const Vector3D& b);
int operator!=(const Vector3D& a, const Vector3D& b);

Vector3D operator+(const Vector3DI& a, const Vector2D& b);
Vector3DI operator+(const Vector3DI& a, const Vector2DI& b);
Vector3D operator+(const Vector3DI& a, const Vector3D& b);
Vector3DI operator+(const Vector3DI& a, const Vector3DI& b);
Vector3D operator-(const Vector3DI& a, const Vector2D& b);
Vector3DI operator-(const Vector3DI& a, const Vector2DI& b);
Vector3D operator-(const Vector3DI& a, const Vector3D& b);
Vector3DI operator-(const Vector3DI& a, const Vector3DI& b);
Vector3DI operator*(const Vector3DI& a, const int& b);
Vector3D operator*(const Vector3DI& a, const float& b);
// pairwise multiplication
Vector3DI operator*(const Vector3DI& a, const Vector3DI& b);
Vector3DI operator/(const Vector3DI& a, const int& b);
Vector3D operator/(const Vector3DI& a, const float& b);
int operator==(const Vector3DI& a, const Vector3DI& b);
int operator!=(const Vector3DI& a, const Vector3DI& b);

Vector2D operator*(const float& a, const Vector2D& b);
Vector2D operator*(const float& a, const Vector2DI& b);
Vector3D operator*(const float& a, const Vector3D& b);
Vector3DI operator*(const float& a, const Vector3DI& b);

Vector2D operator/(const float& a, const Vector2D& b);
Vector2D operator/(const float& a, const Vector2DI& b);
Vector3D operator/(const float& a, const Vector3D& b);
Vector3D operator/(const float& a, const Vector3DI& b);

Vector2DI operator*(const int& a, const Vector2DI& b);
Vector3DI operator*(const int& a, const Vector3DI& b);

inline Vector2D operator*(const Vector2DI& a, const double& b) { return a * float(b); }
inline Vector2D operator/(const Vector2DI& a, const double& b) { return a / float(b); }
inline Vector3D operator*(const Vector3DI& a, const double& b) { return a * float(b); }
inline Vector3D operator/(const Vector3DI& a, const double& b) { return a / float(b); }
inline Vector2D operator*(const double& a, const Vector2DI& b) { return float(a) * b; }
inline Vector3DI operator*(const double& a, const Vector3DI& b) { return float(a) * b; }

GeoRectangle2D operator+(const GeoRectangle2D& a, const Position2D& b);
GeoRectangle2D operator-(const GeoRectangle2D& a, const Position2D& b);
GeoRectangle2D operator*(const GeoRectangle2D& a, const float& b);
GeoRectangle2D operator/(const GeoRectangle2D& a, const float& b);


Vector2D& operator+=(Vector2D& a, const Vector2D& b);
Vector2D& operator+=(Vector2D& a, const Vector2DI& b);
Vector2D& operator-=(Vector2D& a, const Vector2D& b);
Vector2D& operator-=(Vector2D& a, const Vector2DI& b);
Vector2D& operator*=(Vector2D& a, const float& b);
// pairwise multiplication
Vector2D& operator*=(Vector2D& a, const Vector2D& b);
Vector2D& operator/=(Vector2D& a, const float& b);

Vector2DI& operator+=(Vector2DI& a, const Vector2DI& b);
Vector2DI& operator-=(Vector2DI& a, const Vector2DI& b);
Vector2DI& operator*=(Vector2DI& a, const int& b);
// pairwise multiplication
Vector2DI& operator*=(Vector2DI& a, const Vector2DI& b);
Vector2DI& operator/=(Vector2DI& a, const int& b);

Vector3D& operator+=(Vector3D& a, const Vector2D& b);
Vector3D& operator+=(Vector3D& a, const Vector2DI& b);
Vector3D& operator+=(Vector3D& a, const Vector3D& b);
Vector3D& operator+=(Vector3D& a, const Vector3DI& b);
Vector3D& operator-=(Vector3D& a, const Vector2D& b);
Vector3D& operator-=(Vector3D& a, const Vector2DI& b);
Vector3D& operator-=(Vector3D& a, const Vector3D& b);
Vector3D& operator-=(Vector3D& a, const Vector3DI& b);
Vector3D& operator*=(Vector3D& a, const float& b);
// pairwise multiplication
Vector3D& operator*=(Vector3D& a, const Vector3D& b); 
Vector3D& operator*=(Vector3D& a, const Vector2D& b);
Vector3D& operator/=(Vector3D& a, const float& b);

Vector3DI& operator+=(Vector3DI& a, const Vector2DI& b);
Vector3DI& operator+=(Vector3DI& a, const Vector3DI& b);
Vector3DI& operator-=(Vector3DI& a, const Vector2DI& b);
Vector3DI& operator-=(Vector3DI& a, const Vector3DI& b);
Vector3DI& operator*=(Vector3DI& a, const int& b);
// pairwise multiplication
Vector3DI& operator*=(Vector3DI& a, const Vector3DI& b);
Vector3DI& operator/=(Vector3DI& a, const int& b);

GeoRectangle2D& operator+=(GeoRectangle2D& a, const Position2D& b);
GeoRectangle2D& operator-=(GeoRectangle2D& a, const Position2D& b);
GeoRectangle2D& operator*=(GeoRectangle2D& a, const float& b);
GeoRectangle2D& operator/=(GeoRectangle2D& a, const float& b);

GeoRectangle operator*(const GeoRectangle& a, const float& b);
GeoRectangle operator+(const GeoRectangle& a, const Position& b);

}