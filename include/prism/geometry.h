#pragma once

struct Vector3D;
struct Vector3DI;

struct Vector2D {
	inline Vector2D() {};
	Vector2D(double x, double y);
	Vector3D xyz(double z) const;
	double x;
	double y;
};
using Position2D = Vector2D;

struct Vector3D {
	inline Vector3D() {};
	Vector3D(double x, double y, double z);
	Vector3D(const Vector3DI& v);
	Vector2D xy() const;
	double x;
	double y;
	double z;
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
	GeoRectangle2D(double x, double y, double w, double h);
	Position2D mTopLeft;
	Position2D mBottomRight;
};

struct GeoRectangle {
	inline GeoRectangle() {};
	GeoRectangle(double x, double y, double z, double w, double h);
	GeoRectangle2D rect2D() const;
	Position mTopLeft;
	Position mBottomRight;
};

typedef struct {
  Position2D mCenter;
  double mRadius;
} Circle2D;

typedef struct {
	Position mCenter;
	double mRadius;
} Circle;

typedef struct {
	Position2D mP1;
	Position2D mP2;
} Line2D;

typedef struct{
	Position mP1;
	Position mP2;
} Line;

double dot2D(const Vector2D& p1, const Vector2D& p2);
double dot3D(const Vector3D& p1, const Vector3D& p2);

Position variatePosition(const Position& tBase);
void printPosition(char* tName, const Position& tPosition);

double vecLength(const Vector2D& v);
double vecLength(const Vector2DI& v);
double vecLength(const Vector3D& v);
double vecLength(const Vector3DI& v);
Vector3D vecAdd(const Vector3D& v1, const Vector3D& v2);
Vector3D vecSub(const Vector3D& v1, const Vector3D& v2);
Vector3D vecScale(const Vector3D& v, double tFactor);
Vector3D vecScale2D(const Vector3D& v, const Vector2D& tScale);
Vector3D vecScale3D(const Vector3D& v, const Vector3D& tScale);
Vector2D vecNormalize(const Vector2DI& tVector);
Vector2D vecNormalize(const Vector2D& tVector);
Vector3D vecNormalize(const Vector3DI& tVector);
Vector3D vecNormalize(const Vector3D& tVector);
Vector2D vecRotateZ2D(const Vector2D& tVector, double tAngle);
Vector3D vecRotateZ(const Vector3D& tVector, double tAngle);
Vector3D vecRotateZAroundCenter(const Vector3D& tVector, double tAngle, const Vector3D& tCenter);
Vector3D vecScaleToSize(const Vector3D& v, double tSize);
Position getDirection(const Position& tFrom, const Position& tTo);
double getDistance2D(const Position& tFrom, const Position& tTo);
Line2D makeLine2D(const Vector2D& tStart, const Vector2D& tEnd);
Line makeLine(const Vector3D& tStart, const Vector3D& tEnd);

Vector3DI vecAddI(const Vector3DI& v1, const Vector3DI& v2);
Vector3DI vecScaleI(const Vector3DI& v, double tFactor);
Vector2DI vecScaleI2D(const Vector2DI& v, double tFactor);
int vecEqualsI(const Vector3DI& v1, const Vector3DI& v2);
int vecEqualsI2D(const Vector3DI& v1, const Vector3DI& v2);

double vecLength2D(const Vector3D& v);
Vector3D vecAdd2D(const Vector3D& v1, const Vector3D& v2);
Vector3D vecSub2D(const Vector3D& v1, const Vector3D& v2);
Vector3D vecMin2D(const Vector3D& v1, const Vector3D& v2);
Vector3D vecMax2D(const Vector3D& v1, const Vector3D& v2);

Vector2DI vecMinI2D(const Vector2DI& v1, const Vector2DI& v2);
Vector2DI vecMaxI2D(const Vector2DI& v1, const Vector2DI& v2);
Vector3DI vecMinI2D(const Vector3DI& v1, const Vector3DI& v2);
Vector3DI vecMaxI2D(const Vector3DI& v1, const Vector3DI& v2);

double getAngleFromDirection(const Vector3D& tDirection);
double getAngleFromDirection(const Vector2D& tDirection);
Vector3D getDirectionFromAngleZ(double tAngle);
double degreesToRadians(double tDegrees);
double radiansToDegrees(double tRadians);

int checkIntersectLineCircle(const Line2D& tLine, const Circle2D& tCircle);

int checkPointInCircle(const Circle2D& tCirc, const Position2D& tPoint);
int checkPointInRectangle(const GeoRectangle2D& tRect, const Position2D& tPoint);
int checkIntersectCircRect(const Circle2D& tCirc, const GeoRectangle2D& tRect);

Vector2D clampPositionToGeoRectangle(const Vector2D& v, const GeoRectangle2D& tRect);
Vector3D clampPositionToGeoRectangle(const Vector3D& v, const GeoRectangle2D& tRect);
Vector3D clampPositionToGeoRectangle(const Vector3D& v, const GeoRectangle& tRect);
GeoRectangle2D scaleGeoRectangleByFactor(const GeoRectangle2D& tRect, double tFac);
GeoRectangle scaleGeoRectangleByFactor(const GeoRectangle& tRect, double tFac);
GeoRectangle2D scaleGeoRectangleByFactor2D(const GeoRectangle2D& tRect, const Vector2D& tFac);
GeoRectangle2D scaleGeoRectangleByFactor2D(const GeoRectangle2D& tRect, const Vector3D& tFac);
GeoRectangle scaleGeoRectangleByFactor2D(const GeoRectangle& tRect, const Vector2D& tFac);
GeoRectangle scaleGeoRectangleByFactor2D(const GeoRectangle& tRect, const Vector3D& tFac);

Vector3D interpolatePositionLinear(const Position& a, const Position& b, double t);

Vector2D operator+(const Vector2D& a, const Vector2D& b);
Vector2D operator+(const Vector2D& a, const Vector2DI& b);
Vector3D operator+(const Vector3D& a, const Vector2D& b);
Vector2D operator-(const Vector2D& a, const Vector2D& b);
Vector2D operator-(const Vector2D& a, const Vector2DI& b);
Vector3D operator-(const Vector3D& a, const Vector2D& b);
Vector2D operator*(const double& a, const Vector2D& b);
Vector2D operator*(const Vector2D& a, const double& b);
Vector2D operator*(const Vector2D& a, const Vector2D& b);
Vector3D operator*(const Vector3D& a, const Vector2D& b);
Vector2D operator/(const Vector2D& a, const double& b);
Vector2D operator/(const double& a, const Vector2D& b);
int operator==(const Vector2D& a, const Vector2D& b);
int operator!=(const Vector2D& a, const Vector2D& b);

Vector3D operator+(const Vector3D& a, const Vector3D& b);
Vector3D operator-(const Vector3D& a, const Vector3D& b);
Vector3D operator*(const double& a, const Vector3D& b);
Vector3D operator*(const Vector3D& a, const double& b);
void operator*=(Vector3D& a, const double& b);
Vector3D operator/(const Vector3D& a, const double& b);
Vector3D operator/(const double& a, const Vector3D& b);
// pairwise multiplication
Vector3D operator*(const Vector3D& a, const Vector3D& b); 
int operator==(const Vector3D& a, const Vector3D& b);
int operator!=(const Vector3D& a, const Vector3D& b);
Vector3D& operator+=(Vector3D& a, const Vector2D& b);
Vector3D& operator+=(Vector3D& a, const Vector3D& b);
Vector3D& operator-=(Vector3D& a, const Vector2D& b);

Vector2DI operator+(const Vector2DI& a, const Vector2DI& b);
Vector3DI operator+(const Vector3DI& a, const Vector3DI& b);
Vector3D operator+(const Vector3DI& a, const Vector3D& b);
Vector3D operator+(const Vector3D& a, const Vector3DI& b);
Vector2DI operator-(const Vector2DI& a, const Vector2DI& b);
Vector3DI operator-(const Vector3DI& a, const Vector3DI& b);
Vector3D operator-(const Vector3D& a, const Vector3DI& b);
Vector2D operator*(const Vector2DI& a, const double& b);
Vector2D operator/(const Vector2DI& a, const double& b);
Vector3D operator/(const Vector3DI& a, const double& b);
Vector3DI operator/(const Vector3DI& a, const int& b);
// pairwise multiplication
Vector3DI operator*(const Vector3DI& a, const Vector3DI& b);
int operator==(const Vector2DI& a, const Vector2DI& b);
int operator==(const Vector3DI& a, const Vector3DI& b);
int operator!=(const Vector3DI& a, const Vector3DI& b);

GeoRectangle2D operator*(const GeoRectangle2D& a, const double& b);
GeoRectangle2D operator+(const GeoRectangle2D& a, const Position2D& b);
GeoRectangle operator*(const GeoRectangle& a, const double& b);
GeoRectangle operator+(const GeoRectangle& a, const Position& b);
