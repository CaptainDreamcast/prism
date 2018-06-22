#pragma once

typedef struct {
  double x;
  double y;
  double z;
} Vector3D;

typedef struct {
  int x;
  int y;
  int z;
} Vector3DI;

typedef Vector3D Position;

typedef struct {
  Position mTopLeft;
  Position mBottomRight;
} GeoRectangle;

typedef struct {
  Position mCenter;
  double mRadius;
} Circle;

typedef struct{
	Position mP1;
	Position mP2;
} Line;


double dot3D(Vector3D p1, Vector3D p2);

Vector3DI makeVector3DI(int x, int y, int z);
Position makePosition(double x, double y, double z);
Position variatePosition(Position tBase);
void printPosition(char* tName, Position tPosition);

double vecLength(Vector3D v);
Vector3D vecAdd(Vector3D v1, Vector3D v2);
Vector3D vecSub(Vector3D v1, Vector3D v2);
Vector3D vecScale(Vector3D v, double tFactor);
Vector3D vecScale3D(Vector3D v, Vector3D tScale);
Vector3D vecNormalize(Vector3D tVector);
Vector3D vecRotateZ(Vector3D tVector, double tAngle);
Vector3D vecRotateZAroundCenter(Vector3D tVector, double tAngle, Vector3D tCenter);
Vector3D vecScaleToSize(Vector3D v, double tSize);
Position getDirection(Position tFrom, Position tTo);
double getDistance2D(Position tFrom, Position tTo);
Line makeLine(Vector3D tStart, Vector3D tEnd);
GeoRectangle makeGeoRectangle(double x, double y, double w, double h);
GeoRectangle makeGeoRectangle3D(double x, double y, double z, double w, double h);

Vector3DI vecAddI(Vector3DI v1, Vector3DI v2);
int vecEqualsI(Vector3DI v1, Vector3DI v2);

double vecLength2D(Vector3D v);
Vector3D vecAdd2D(Vector3D v1, Vector3D v2);
Vector3D vecSub2D(Vector3D v1, Vector3D v2);

double getAngleFromDirection(Vector3D tDirection);
Vector3D getDirectionFromAngleZ(double tAngle);
double degreesToRadians(double tDegrees);

int checkIntersectLineCircle(Line tLine, Circle tCircle);

int checkPointInCircle(Circle tCirc, Position tPoint);
int checkPointInRectangle(GeoRectangle tRect, Position tPoint);
int checkIntersectCircRect(Circle tCirc, GeoRectangle tRect);

Vector3D clampPositionToGeoRectangle(Vector3D v, GeoRectangle tRect);
GeoRectangle scaleGeoRectangleByFactor(GeoRectangle tRect, double tFac);
GeoRectangle scaleGeoRectangleByFactor2D(GeoRectangle tRect, Vector3D tFac);

Vector3D interpolatePositionLinear(Position a, Position b, double t);
