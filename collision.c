#include "include/collision.h"

#include "include/log.h"

// TODO: use something better; this will likely cause vibrations
void resolveCollsion(PhysicsObject* tObject, CollisionRect tObjectRect, CollisionRect tOtherRect) {
  Velocity vel = tObject->mVelocity;

  if (isEmptyVelocity(vel)) {
    logError("Resolving collision with no velocity");
    vel.x = 1;
  } else {
    vel = normalizeVelocity(vel);
  }

  Position delta;
  delta.x = delta.y = delta.z = 0;
  CollisionRect temp = tObjectRect;

  while (checkCollision(temp, tOtherRect)) {
    delta.x -= vel.x;
    delta.y -= vel.y;
    delta.z -= vel.z;

    temp.mTopLeft.x -= vel.x;
    temp.mBottomRight.x -= vel.x;
    temp.mTopLeft.y -= vel.y;
    temp.mBottomRight.y -= vel.y;
    temp.mTopLeft.z -= vel.z;
    temp.mBottomRight.z -= vel.z;
  }

  tObject->mPosition.x += delta.x;
  tObject->mPosition.y += delta.x;
  tObject->mPosition.z += delta.x;
}

int checkCollision(CollisionRect tRect1, CollisionRect tRect2) {
  if (tRect1.mTopLeft.x > tRect2.mBottomRight.x)
    return 0;
  if (tRect1.mTopLeft.y < tRect2.mBottomRight.y)
    return 0;
  if (tRect2.mTopLeft.x > tRect1.mBottomRight.x)
    return 0;
  if (tRect2.mTopLeft.y < tRect1.mBottomRight.y)
    return 0;

  debugLog("Collision found");
  return 1;
}


int checkCollisionCirc(CollisionCirc tCirc1, CollisionCirc tCirc2){
	Vector3D delta;
	delta.x = tCirc1.mCenter.x - tCirc2.mCenter.x;
	delta.y = tCirc1.mCenter.y - tCirc2.mCenter.y;
	delta.z = tCirc1.mCenter.z - tCirc2.mCenter.z;
	double l = vecLength(delta);

	return l < tCirc1.mRadius+tCirc2.mRadius;
}


CollisionObjectRect makeCollisionObjectRect(Position tTopLeft, Position tBottomRight, PhysicsObject* tPhysics){
	CollisionObjectRect ret;

	ret.mIsPositionInColRelative = 1;	
	ret.mCol.mTopLeft = tTopLeft; 
	ret.mCol.mBottomRight = tBottomRight;
	ret.mPhysics = tPhysics;

	return ret;
}

CollisionObjectCirc makeCollisionObjectCirc(Position tCenter, double tRadius, PhysicsObject* tPhysics){
	CollisionObjectCirc ret;

	ret.mIsPositionInColRelative = 1;	
	ret.mCol.mCenter = tCenter; 
	ret.mCol.mRadius = tRadius;
	ret.mPhysics = tPhysics;

	return ret;


}

int checkCollisionObjectCirc(CollisionObjectCirc tObj1, CollisionObjectCirc tObj2){

	CollisionCirc c1 = tObj1.mCol;	
	CollisionCirc c2 = tObj2.mCol;	

	if(tObj1.mIsPositionInColRelative) {
		c1.mCenter = vecAdd(tObj1.mCol.mCenter, tObj1.mPhysics->mPosition);
	}

	if(tObj2.mIsPositionInColRelative) {
		c2.mCenter = vecAdd(tObj1.mCol.mCenter, tObj1.mPhysics->mPosition);
	}

	return checkCollisionCirc(c1, c2);
}
