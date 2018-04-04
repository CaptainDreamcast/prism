#include "prism/collision.h"

#include <string.h>

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"

Vector3D getColliderColliderMovableStaticDelta(Velocity tVel1, Collider tCollider1, Collider tCollider2) {
	if (isEmptyVelocity(tVel1)) return makePosition(INF, INF, 0);

	Position tempPos = *tCollider1.mBasePosition;
	Collider temp = tCollider1;
	temp.mBasePosition = &tempPos;
	Position delta = makePosition(0, 0, 0);
	while (checkCollisionCollider(temp, tCollider2)) {
		delta = vecSub(delta, tVel1);
		tempPos = vecSub(tempPos, tVel1);
	}
	return delta;
}

void resolveCollisionColliderColliderMovableStatic(Position* tPos1, Velocity tVel1, Collider tCollider1, Collider tCollider2) {
	if (isEmptyVelocity(tVel1)) return;
	double scale = 0.01;
	tVel1 = vecScale(normalizeVelocity(tVel1), scale);
	
	Position deltas[8];
	deltas[0] = getColliderColliderMovableStaticDelta(tVel1, tCollider1, tCollider2);
	deltas[1] = getColliderColliderMovableStaticDelta(vecScale(tVel1, -1), tCollider1, tCollider2);
	deltas[2] = getColliderColliderMovableStaticDelta(makePosition(-tVel1.x, tVel1.y, 0), tCollider1, tCollider2);
	deltas[3] = getColliderColliderMovableStaticDelta(makePosition(tVel1.x, -tVel1.y, 0), tCollider1, tCollider2);
	deltas[4] = getColliderColliderMovableStaticDelta(makePosition(scale, 0, 0), tCollider1, tCollider2);
	deltas[5] = getColliderColliderMovableStaticDelta(makePosition(-scale, 0, 0), tCollider1, tCollider2);
	deltas[6] = getColliderColliderMovableStaticDelta(makePosition(0, scale, 0), tCollider1, tCollider2);
	deltas[7] = getColliderColliderMovableStaticDelta(makePosition(0, -scale, 0), tCollider1, tCollider2);

	int i;
	int smallestIndex = 0;
	for (i = 1; i < 8; i++) {
		if (vecLength(deltas[i]) < vecLength(deltas[smallestIndex])) {
			smallestIndex = i;
		}
	}
	
	*tPos1 = vecAdd(*tPos1, deltas[smallestIndex]);
}

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

  if(tRect1.mTopLeft.x > tRect1.mBottomRight.x) return 0;
  if(tRect1.mTopLeft.y > tRect1.mBottomRight.y) return 0;
  if(tRect2.mTopLeft.x > tRect2.mBottomRight.x) return 0;
  if(tRect2.mTopLeft.y > tRect2.mBottomRight.y) return 0;

  if (tRect1.mTopLeft.x > tRect2.mBottomRight.x)
    return 0;
  if (tRect1.mTopLeft.y > tRect2.mBottomRight.y)
    return 0;
  if (tRect2.mTopLeft.x > tRect1.mBottomRight.x)
    return 0;
  if (tRect2.mTopLeft.y > tRect1.mBottomRight.y)
    return 0;

  // TODO: fix for Hazy Hank

  debugLog("Collision found");
  return 1;
}


int checkCollisionCirc(CollisionCirc tCirc1, CollisionCirc tCirc2){
	Vector3D delta;
	delta.x = tCirc1.mCenter.x - tCirc2.mCenter.x;
	delta.y = tCirc1.mCenter.y - tCirc2.mCenter.y;
	delta.z = 0;
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

static void adjustRectByPosition(CollisionRect* tRect, Position tPos) {
	tRect->mTopLeft = vecAdd(tRect->mTopLeft, tPos);
	tRect->mBottomRight = vecAdd(tRect->mBottomRight, tPos);
}

static void adjustCircByPosition(CollisionCirc* tCirc, Position tPos) {
	tCirc->mCenter = vecAdd(tCirc->mCenter, tPos);
}


int checkCollisionCollider(Collider tCollider1, Collider tCollider2) {
	if(tCollider1.mType == COLLISION_RECT && tCollider2.mType == COLLISION_RECT) {
		CollisionRect r1 = *((CollisionRect*)tCollider1.mData);
		CollisionRect r2 = *((CollisionRect*)tCollider2.mData);
		adjustRectByPosition(&r1, *tCollider1.mBasePosition);
		adjustRectByPosition(&r2, *tCollider2.mBasePosition);
		return checkCollision(r1, r2);
	} else if(tCollider1.mType == COLLISION_CIRC && tCollider2.mType == COLLISION_RECT) {
		CollisionCirc c1 = *((CollisionCirc*)tCollider1.mData);
		CollisionRect r2 = *((CollisionRect*)tCollider2.mData);
		adjustCircByPosition(&c1, *tCollider1.mBasePosition);
		adjustRectByPosition(&r2, *tCollider2.mBasePosition);
		return checkCollisionCircRect(c1, r2);
	} else if(tCollider1.mType == COLLISION_RECT && tCollider2.mType == COLLISION_CIRC) {
		return checkCollisionCollider(tCollider2, tCollider1);
	}  else if(tCollider1.mType == COLLISION_CIRC && tCollider2.mType == COLLISION_CIRC) {
		CollisionCirc c1 = *((CollisionCirc*)tCollider1.mData);
		CollisionCirc c2 = *((CollisionCirc*)tCollider2.mData);
		adjustCircByPosition(&c1, *tCollider1.mBasePosition);
		adjustCircByPosition(&c2, *tCollider2.mBasePosition);
		return checkCollisionCirc(c1, c2);
	} else {
		logError("Unrecognized collider types");			
		logErrorInteger(tCollider1.mType);
		logErrorInteger(tCollider2.mType);
		abortSystem();
		#ifdef DREAMCAST
		return 0; // TODO: fix unreachable code (Windows) / no return (DC) conflict
		#endif
	}

}

static Collider makeCollider_internal(int tType, void* tData, int tDataSize) {
	Collider ret;
	ret.mType = tType;
	ret.mData = allocMemory(tDataSize);
	memcpy(ret.mData, tData, tDataSize);
	ret.mBasePosition = NULL;
	return ret;
}

Collider makeColliderFromRect(CollisionRect tRect) {
	return makeCollider_internal(COLLISION_RECT, &tRect, sizeof(CollisionRect));
}

Collider makeColliderFromCirc(CollisionCirc tCirc) {
	return makeCollider_internal(COLLISION_CIRC, &tCirc, sizeof(CollisionCirc));
}

void setColliderBasePosition(Collider* tCollider, Position* tBasePosition) {
	tCollider->mBasePosition = tBasePosition;
}

void destroyCollider(Collider* tCollider) {
	freeMemory(tCollider->mData);
}

double getColliderUp(Collider tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		CollisionRect* rect = tCollider.mData;
		pos = vecAdd(pos, rect->mTopLeft);
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		CollisionCirc* circ = tCollider.mData;
		pos = vecAdd(pos, circ->mCenter);
		pos.y -= circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		abortSystem();
	}

	return pos.y;
}

double getColliderDown(Collider tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		CollisionRect* rect = tCollider.mData;
		pos = vecAdd(pos, rect->mBottomRight);
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		CollisionCirc* circ = tCollider.mData;
		pos = vecAdd(pos, circ->mCenter);
		pos.y += circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		abortSystem();
	}

	return pos.y;
}

double getColliderRight(Collider tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		CollisionRect* rect = tCollider.mData;
		pos = vecAdd(pos, rect->mBottomRight);
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		CollisionCirc* circ = tCollider.mData;
		pos = vecAdd(pos, circ->mCenter);
		pos.x += circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		abortSystem();
	}

	return pos.x;
}

double getColliderLeft(Collider tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		CollisionRect* rect = tCollider.mData;
		pos = vecAdd(pos, rect->mTopLeft);
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		CollisionCirc* circ = tCollider.mData;
		pos = vecAdd(pos, circ->mCenter);
		pos.x -= circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		abortSystem();
	}

	return pos.x;
}

