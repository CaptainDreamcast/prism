#pragma once

#include "physics.h"
#include "geometry.h"

typedef GeoRectangle2D CollisionRect;
typedef Circle2D CollisionCirc;

typedef enum {
	COLLISION_CIRC,
	COLLISION_RECT
} CollisionType;

typedef struct {
	CollisionCirc mCol;
	PhysicsObject* mPhysics;
	int mIsPositionInColRelative;
} CollisionObjectCirc;

typedef struct {
	CollisionRect mCol;
	PhysicsObject* mPhysics;
	int mIsPositionInColRelative;
} CollisionObjectRect;

typedef struct Collider_t {
	CollisionType mType;
	Position* mBasePosition;

	operator const CollisionCirc*() const {
		return &mImpl.mCirc;
	}

	operator const CollisionRect*() const {
		return &mImpl.mRect;
	}

	union Impl{
		CollisionCirc mCirc;
		CollisionRect mRect;
		Impl() {}
	} mImpl;
} Collider;

void resolveCollisionColliderColliderMovableStatic(Position* tPos1, const Velocity& tVel1, const Collider& tCollider1, const Collider& tCollider2);
int checkCollision(const CollisionRect& tRect1, const CollisionRect& tRect2);
int checkCollisionCirc(const CollisionCirc& tCirc1, const CollisionCirc& tCirc2);
int checkCollisionCircRect(const CollisionCirc& tCirc, const CollisionRect& tRect);
int checkCollisionCollider(const Collider& tCollider1, const Collider& tCollider2);

CollisionObjectRect makeCollisionObjectRect(const Position2D& tTopLeft, const Position2D& tBottomRight, PhysicsObject* tPhysics);
CollisionObjectCirc makeCollisionObjectCirc(const Position2D& tCenter, double tRadius, PhysicsObject* tPhysics);
CollisionRect adjustCollisionObjectRect(CollisionObjectRect* tObj);
CollisionCirc adjustCollisionObjectCirc(CollisionObjectCirc* tObj);


CollisionRect makeCollisionRect(const Position2D& tTopLeft, const Position2D& tBottomRight);
CollisionCirc makeCollisionCirc(const Position& tCenter, double tRadius);

int checkCollisionObjectCirc(const CollisionObjectCirc& tObj1, const CollisionObjectCirc& tObj2);
int checkCollisionObjectRect(const CollisionObjectRect& tObj1, const CollisionObjectRect& tObj2);
int checkCollisionObjectCircRect(const CollisionObjectCirc& tObj1, const CollisionObjectRect& tObj2);

Collider makeColliderFromRect(const CollisionRect& tRect);
Collider makeColliderFromCirc(const CollisionCirc& tCirc);
void setColliderBasePosition(Collider* tCollider, Position* tBasePosition);
void destroyCollider(Collider* tCollider);

double getColliderUp(const Collider& tCollider);
double getColliderDown(const Collider& tCollider);
double getColliderRight(const Collider& tCollider);
double getColliderLeft(const Collider& tCollider);
