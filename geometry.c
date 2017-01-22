#include "include/geometry.h"

#include <kos.h>

int dot3D(Vector3D p1, Vector3D p2){
	return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
}

double vecLength(Vector3D tVelocity) {
  return fsqrt(tVelocity.x * tVelocity.x + tVelocity.y * tVelocity.y + tVelocity.z * tVelocity.z);
}

Vector3D vecAdd(Vector3D v1, Vector3D v2){
	Vector3D ret;
	ret.x = v1.x+v2.x;
	ret.y = v1.y+v2.y;
	ret.z = v1.z+v2.z;
	return ret;
}

Vector3D vecScale(Vector3D v, double tFactor){
	Vector3D ret;
	ret.x = v.x*tFactor;
	ret.y = v.y*tFactor;
	ret.z = v.z*tFactor;
	return ret;
}

Position makePosition(double x, double y, double z){
  Position pos;
  pos.x = x;
  pos.y = y;
  pos.z = z;
  return pos;
}

Position getDirection(Position tFrom, Position tTo){
	Position ret;
	ret.x = tTo.x-tFrom.x;
	ret.y = tTo.y-tFrom.y;
	ret.z = tTo.z-tFrom.z;
	return ret;
}

Line makeLine(Vector3D tStart, Vector3D tEnd){
	Line ret;
	ret.mP1 = tStart;
	ret.mP2 = tEnd;
	return ret;
}

// TODO: refactor and/or add intersection position test;
int checkIntersectLineCircle(Line tLine, Circle tCircle){

	float r = tCircle.mRadius;

	Position d = getDirection(tLine.mP1, tLine.mP2);
	Position f = getDirection(tCircle.mCenter, tLine.mP1);

	float a = dot3D(d, d) ;
	float b = 2*dot3D(f,d) ;
	float c = dot3D(f, f) - r*r ;

	float discriminant = b*b-4*a*c;
	if( discriminant < 0 )
	{
		return 0;
	}
	else
	{
		// ray didn't totally miss sphere,
		// so there is a solution to
		// the equation.

		discriminant = fsqrt( discriminant );

		// either solution may be on or off the ray so need to test both
		// t1 is always the smaller value, because BOTH discriminant and
		// a are nonnegative.
		float t1 = (-b - discriminant)/(2*a);
		float t2 = (-b + discriminant)/(2*a);

		// 3x HIT cases:
		//          -o->             --|-->  |            |  --|->
		// Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

		// 3x MISS cases:
		//       ->  o                     o ->              | -> |
		// FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

		if( t1 >= 0 && t1 <= 1 )
		{
			// t1 is the intersection, and it's closer than t2
			// (since t1 uses -b - discriminant)
			// Impale, Poke
			return 1 ;
		}

		// here t1 didn't intersect so we are either started
		// inside the sphere or completely past it
		if( t2 >= 0 && t2 <= 1 )
		{
			// ExitWound
			return 1 ;
		}

		// no intn: FallShort, Past, CompletelyInside
		return 0 ;
	}
}

int checkPointInCircle(Circle tCirc, Position tPoint){
	Position d = getDirection(tCirc.mCenter, tPoint);
	double l = vecLength(d);
	return l <= tCirc.mRadius;
}

int checkPointInRectangle(GeoRectangle tRect, Position tPoint){
	return tPoint.x >= tRect.mTopLeft.x && tPoint.x <= tRect.mBottomRight.x && tPoint.y >= tRect.mTopLeft.y && tPoint.y <= tRect.mBottomRight.y;
}

int checkIntersectCircRect(Circle tCirc, GeoRectangle tRect){
	Position p1 = tRect.mTopLeft;
	Position p2 = makePosition(tRect.mTopLeft.x, tRect.mBottomRight.y, 0);
	Position p3 = tRect.mBottomRight;
	Position p4 = makePosition(tRect.mBottomRight.x, tRect.mTopLeft.y, 0);
	
	return (checkPointInRectangle(tRect, tCirc.mCenter) ||
            checkIntersectLineCircle(makeLine(p1, p2), tCirc) ||
            checkIntersectLineCircle(makeLine(p2, p3), tCirc) ||
            checkIntersectLineCircle(makeLine(p3, p4), tCirc) ||
            checkIntersectLineCircle(makeLine(p4, p1), tCirc));

}
