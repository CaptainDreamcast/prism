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

int checkCollisionCircRect(CollisionCirc tCirc, CollisionRect tRect){
	return checkIntersectCircRect(tCirc, tRect);
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

CollisionRect makeCollisionRect(Position tTopLeft, Position tBottomRight){
	CollisionRect ret;
	ret.mTopLeft = tTopLeft; 
	ret.mBottomRight = tBottomRight;
	return ret;
}

CollisionCirc makeCollisionCirc(Position tCenter, double tRadius){
	CollisionCirc ret;
	ret.mCenter = tCenter; 
	ret.mRadius = tRadius;
	return ret;
}

CollisionCirc adjustCollisionObjectCirc(CollisionObjectCirc* tObj){
	CollisionCirc c = tObj->mCol;
	if(tObj->mIsPositionInColRelative) {
		c.mCenter = vecAdd(tObj->mCol.mCenter, tObj->mPhysics->mPosition);
	}
	return c;
}

int checkCollisionObjectCirc(CollisionObjectCirc tObj1, CollisionObjectCirc tObj2){

	CollisionCirc c1 = adjustCollisionObjectCirc(&tObj1);
	CollisionCirc c2 = adjustCollisionObjectCirc(&tObj2);

	return checkCollisionCirc(c1, c2);
}

CollisionRect adjustCollisionObjectRect(CollisionObjectRect* tObj){
	CollisionRect c = tObj->mCol;
	if(tObj->mIsPositionInColRelative) {
		c.mTopLeft = vecAdd(tObj->mCol.mTopLeft, tObj->mPhysics->mPosition);
		c.mBottomRight = vecAdd(tObj->mCol.mBottomRight, tObj->mPhysics->mPosition);
	}
	return c;
}

int checkCollisionObjectRect(CollisionObjectRect tObj1, CollisionObjectRect tObj2){

	CollisionRect c1 = adjustCollisionObjectRect(&tObj1);
	CollisionRect c2 = adjustCollisionObjectRect(&tObj2);

	return checkCollision(c1, c2);
}


int checkCollisionObjectCircRect(CollisionObjectCirc tObj1, CollisionObjectRect tObj2){

	CollisionCirc c1 = adjustCollisionObjectCirc(&tObj1);
	CollisionRect c2 = adjustCollisionObjectRect(&tObj2);

	return checkCollisionCircRect(c1, c2);
}
