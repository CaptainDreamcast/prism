#ifndef TARI_GEOMETRY
#define TARI_GEOMETRY

#include "common/header.h"

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


fup double dot3D(Vector3D p1, Vector3D p2);

fup Vector3DI makeVector3DI(int x, int y, int z);
fup Position makePosition(double x, double y, double z);
fup Position variatePosition(Position tBase);
fup void printPosition(char* tName, Position tPosition);

fup double vecLength(Vector3D v);
fup Vector3D vecAdd(Vector3D v1, Vector3D v2);
fup Vector3D vecScale(Vector3D v, double tFactor);
fup Vector3D vecNormalize(Vector3D tVector);
fup Vector3D vecRotateZ(Vector3D, double tAngle);
fup Position getDirection(Position tFrom, Position tTo);
fup Line makeLine(Vector3D tStart, Vector3D tEnd);

fup double getAngleFromDirection(Vector3D tDirection);

fup int checkIntersectLineCircle(Line tLine, Circle tCircle);

fup int checkPointInCircle(Circle tCirc, Position tPoint);
fup int checkPointInRectangle(GeoRectangle tRect, Position tPoint);
fup int checkIntersectCircRect(Circle tCirc, GeoRectangle tRect);


#endif
