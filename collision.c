#include "include/collision.h"

#include "include/log.h"

// TODO: use something better; this will likely cause vibrations
void resolveCollsion(PhysicsObject* tObject, CollisionRect tObjectRect, CollisionRect tOtherRect){
	Velocity vel = tObject->mVelocity;

	if(isEmptyVelocity(vel)){
		logError("Resolving collision with no velocity");
		vel.x = 1;
	}
	else {
		vel = normalizeVelocity(vel);
	}

	Position delta;
	delta.x = delta.y = delta.z = 0;
	CollisionRect temp = tObjectRect;

	while(checkCollision(temp, tOtherRect)){
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

int checkCollision(CollisionRect tRect1, CollisionRect tRect2){
	if(tRect1.mTopLeft.x > tRect2.mBottomRight.x) return 0;
	if(tRect1.mTopLeft.y < tRect2.mBottomRight.y) return 0;
	if(tRect2.mTopLeft.x > tRect1.mBottomRight.x) return 0;
	if(tRect2.mTopLeft.y < tRect1.mBottomRight.y) return 0;

	debugLog("Collision found");
	return 1;
}

