#ifndef TARI_GEOMETRY
#define TARI_GEOMETRY

typedef struct {
  double x;
  double y;
  double z;
} Vector3D;

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


int dot3D(Vector3D p1, Vector3D p2);

Position makePosition(double x, double y, double z);
double vecLength(Vector3D v);
Vector3D vecAdd(Vector3D v1, Vector3D v2);
Vector3D vecScale(Vector3D v, double tFactor);
Position getDirection(Position tFrom, Position tTo);
Line makeLine(Vector3D tStart, Vector3D tEnd);

int checkIntersectLineCircle(Line tLine, Circle tCircle);

int checkPointInCircle(Circle tCirc, Position tPoint);
int checkPointInRectangle(GeoRectangle tRect, Position tPoint);
int checkIntersectCircRect(Circle tCirc, GeoRectangle tRect);
#endif
