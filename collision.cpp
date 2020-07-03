#include "prism/collision.h"

#include <string.h>

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"

static Vector3D getColliderColliderMovableStaticDelta(const Velocity& tVel1, const Collider& tCollider1, const Collider& tCollider2) {
	if (isEmptyVelocity(tVel1)) return Vector3D(INF, INF, 0);

	Position tempPos = *tCollider1.mBasePosition;
	Collider temp = tCollider1;
	temp.mBasePosition = &tempPos;
	Position delta = Vector3D(0, 0, 0);
	while (checkCollisionCollider(temp, tCollider2)) {
		delta = vecSub(delta, tVel1);
		tempPos = vecSub(tempPos, tVel1);
	}
	return delta;
}

void resolveCollisionColliderColliderMovableStatic(Position* tPos1, const Velocity& tVel1, const Collider& tCollider1, const Collider& tCollider2) {
	if (isEmptyVelocity(tVel1)) return;
	double scale = 0.01;
	const auto scaledVel = vecScale(normalizeVelocity(tVel1), scale);
	
	Position deltas[8];
	deltas[0] = getColliderColliderMovableStaticDelta(scaledVel, tCollider1, tCollider2);
	deltas[1] = getColliderColliderMovableStaticDelta(vecScale(scaledVel, -1), tCollider1, tCollider2);
	deltas[2] = getColliderColliderMovableStaticDelta(Vector3D(-scaledVel.x, scaledVel.y, 0), tCollider1, tCollider2);
	deltas[3] = getColliderColliderMovableStaticDelta(Vector3D(scaledVel.x, -scaledVel.y, 0), tCollider1, tCollider2);
	deltas[4] = getColliderColliderMovableStaticDelta(Vector3D(scale, 0, 0), tCollider1, tCollider2);
	deltas[5] = getColliderColliderMovableStaticDelta(Vector3D(-scale, 0, 0), tCollider1, tCollider2);
	deltas[6] = getColliderColliderMovableStaticDelta(Vector3D(0, scale, 0), tCollider1, tCollider2);
	deltas[7] = getColliderColliderMovableStaticDelta(Vector3D(0, -scale, 0), tCollider1, tCollider2);

	int i;
	int smallestIndex = 0;
	for (i = 1; i < 8; i++) {
		if (vecLength(deltas[i]) < vecLength(deltas[smallestIndex])) {
			smallestIndex = i;
		}
	}
	
	*tPos1 = vecAdd(*tPos1, deltas[smallestIndex]);
}

int checkCollision(const CollisionRect& tRect1, const CollisionRect& tRect2) {

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

  debugLog("Collision found");
  return 1;
}


int checkCollisionCirc(const CollisionCirc& tCirc1, const CollisionCirc& tCirc2){
	Vector3D delta;
	delta.x = tCirc1.mCenter.x - tCirc2.mCenter.x;
	delta.y = tCirc1.mCenter.y - tCirc2.mCenter.y;
	delta.z = 0;
	double l = vecLength(delta);

	return l < tCirc1.mRadius+tCirc2.mRadius;
}

int checkCollisionCircRect(const CollisionCirc& tCirc, const CollisionRect& tRect){
	return checkIntersectCircRect(tCirc, tRect);
}

CollisionObjectRect makeCollisionObjectRect(const Position2D& tTopLeft, const Position2D& tBottomRight, PhysicsObject* tPhysics){
	CollisionObjectRect ret;

	ret.mIsPositionInColRelative = 1;	
	ret.mCol.mTopLeft = tTopLeft; 
	ret.mCol.mBottomRight = tBottomRight;
	ret.mPhysics = tPhysics;

	return ret;
}

CollisionObjectCirc makeCollisionObjectCirc(const Position2D& tCenter, double tRadius, PhysicsObject* tPhysics){
	CollisionObjectCirc ret;

	ret.mIsPositionInColRelative = 1;	
	ret.mCol.mCenter = tCenter; 
	ret.mCol.mRadius = tRadius;
	ret.mPhysics = tPhysics;

	return ret;


}

CollisionRect makeCollisionRect(const Position2D& tTopLeft, const Position2D& tBottomRight){
	CollisionRect ret;
	ret.mTopLeft = tTopLeft; 
	ret.mBottomRight = tBottomRight;
	return ret;
}

CollisionCirc makeCollisionCirc(const Position2D& tCenter, double tRadius){
	CollisionCirc ret;
	ret.mCenter = tCenter; 
	ret.mRadius = tRadius;
	return ret;
}

CollisionCirc adjustCollisionObjectCirc(const CollisionObjectCirc* tObj){
	CollisionCirc c = tObj->mCol;
	if(tObj->mIsPositionInColRelative) {
		c.mCenter = tObj->mCol.mCenter + tObj->mPhysics->mPosition.xy();
	}
	return c;
}

int checkCollisionObjectCirc(const CollisionObjectCirc& tObj1, const CollisionObjectCirc& tObj2){

	const auto c1 = adjustCollisionObjectCirc(&tObj1);
	const auto c2 = adjustCollisionObjectCirc(&tObj2);

	return checkCollisionCirc(c1, c2);
}

CollisionRect adjustCollisionObjectRect(const CollisionObjectRect* tObj){
	CollisionRect c = tObj->mCol;
	if(tObj->mIsPositionInColRelative) {
		c.mTopLeft = tObj->mCol.mTopLeft + tObj->mPhysics->mPosition.xy();
		c.mBottomRight = tObj->mCol.mBottomRight + tObj->mPhysics->mPosition.xy();
	}
	return c;
}

int checkCollisionObjectRect(const CollisionObjectRect& tObj1, const CollisionObjectRect& tObj2){

	const auto c1 = adjustCollisionObjectRect(&tObj1);
	const auto c2 = adjustCollisionObjectRect(&tObj2);

	return checkCollision(c1, c2);
}


int checkCollisionObjectCircRect(const CollisionObjectCirc& tObj1, const CollisionObjectRect& tObj2){

	const auto c1 = adjustCollisionObjectCirc(&tObj1);
	const auto c2 = adjustCollisionObjectRect(&tObj2);

	return checkCollisionCircRect(c1, c2);
}

static void adjustRectByPosition(CollisionRect* tRect, const Position& tPos) {
	tRect->mTopLeft = tRect->mTopLeft + tPos.xy();
	tRect->mBottomRight = tRect->mBottomRight + tPos.xy();
}

static void adjustCircByPosition(CollisionCirc* tCirc, const Position& tPos) {
	tCirc->mCenter = tCirc->mCenter + tPos.xy();
}

int checkCollisionCollider(const Collider& tCollider1, const Collider& tCollider2) {
	if(tCollider1.mType == COLLISION_RECT && tCollider2.mType == COLLISION_RECT) {
		CollisionRect r1 = *((const CollisionRect*)tCollider1);
		CollisionRect r2 = *((const CollisionRect*)tCollider2);
		adjustRectByPosition(&r1, *tCollider1.mBasePosition);
		adjustRectByPosition(&r2, *tCollider2.mBasePosition);
		return checkCollision(r1, r2);
	} else if(tCollider1.mType == COLLISION_CIRC && tCollider2.mType == COLLISION_RECT) {
		CollisionCirc c1 = *((const CollisionCirc*)tCollider1);
		CollisionRect r2 = *((const CollisionRect*)tCollider2);
		adjustCircByPosition(&c1, *tCollider1.mBasePosition);
		adjustRectByPosition(&r2, *tCollider2.mBasePosition);
		return checkCollisionCircRect(c1, r2);
	} else if(tCollider1.mType == COLLISION_RECT && tCollider2.mType == COLLISION_CIRC) {
		return checkCollisionCollider(tCollider2, tCollider1);
	}  else if(tCollider1.mType == COLLISION_CIRC && tCollider2.mType == COLLISION_CIRC) {
		CollisionCirc c1 = *((const CollisionCirc*)tCollider1);
		CollisionCirc c2 = *((const CollisionCirc*)tCollider2);
		adjustCircByPosition(&c1, *tCollider1.mBasePosition);
		adjustCircByPosition(&c2, *tCollider2.mBasePosition);
		return checkCollisionCirc(c1, c2);
	} else {
		logError("Unrecognized collider types");			
		logErrorInteger(tCollider1.mType);
		logErrorInteger(tCollider2.mType);
		recoverFromError();
		return 0;
	}

}

Collider makeColliderFromRect(const CollisionRect& tRect) {
	Collider c;
	c.mType = COLLISION_RECT;
	c.mBasePosition = NULL;
	c.mImpl.mRect = tRect;
	return c;
}

Collider makeColliderFromCirc(const CollisionCirc& tCirc) {
	Collider c;
	c.mType = COLLISION_CIRC;
	c.mBasePosition = NULL;
	c.mImpl.mCirc = tCirc;
	return c;
}

void setColliderBasePosition(Collider* tCollider, Position* tBasePosition) {
	tCollider->mBasePosition = tBasePosition;
}

void destroyCollider(Collider* tCollider) {
	(void)tCollider;
}

double getColliderUp(const Collider& tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		const auto rect = (const CollisionRect*)tCollider;
		pos = pos + rect->mTopLeft;
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		const auto circ = (const CollisionCirc*)tCollider;
		pos = pos + circ->mCenter;
		pos.y -= circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		recoverFromError();
	}

	return pos.y;
}

double getColliderDown(const Collider& tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		const auto rect = (const CollisionRect*)tCollider;
		pos = pos + rect->mBottomRight;
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		const auto circ = (const CollisionCirc*)tCollider;
		pos = pos + circ->mCenter;
		pos.y += circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		recoverFromError();
	}

	return pos.y;
}

double getColliderRight(const Collider& tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		const auto rect = (const CollisionRect*)tCollider;
		pos = pos + rect->mBottomRight;
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		const auto circ = (const CollisionCirc*)tCollider;
		pos = pos + circ->mCenter;
		pos.x += circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		recoverFromError();
	}

	return pos.x;
}

double getColliderLeft(const Collider& tCollider)
{
	Position pos = *tCollider.mBasePosition;
	if (tCollider.mType == COLLISION_RECT) {
		const auto rect = (const CollisionRect*)tCollider;
		pos = pos + rect->mTopLeft;
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		const auto circ = (const CollisionCirc*)tCollider;
		pos = pos + circ->mCenter;
		pos.x -= circ->mRadius;
	}
	else {
		logErrorFormat("Unrecognized type: %d.", tCollider.mType);
		recoverFromError();
	}

	return pos.x;
}

