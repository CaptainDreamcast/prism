#ifndef TARI_GEOMETRY
#define TARI_GEOMETRY

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
Vector3D vecScale(Vector3D v, double tFactor);
Vector3D vecNormalize(Vector3D tVector);
Position getDirection(Position tFrom, Position tTo);
Line makeLine(Vector3D tStart, Vector3D tEnd);

double getAngleFromDirection(Vector3D tDirection);

int checkIntersectLineCircle(Line tLine, Circle tCircle);

int checkPointInCircle(Circle tCirc, Position tPoint);
int checkPointInRectangle(GeoRectangle tRect, Position tPoint);
int checkIntersectCircRect(Circle tCirc, GeoRectangle tRect);


#endif
