#ifndef TARI_COLLISION
#define TARI_COLLISION

#include "physics.h"

typedef struct {
  Position mTopLeft;
  Position mBottomRight;
} CollisionRect;

typedef struct {
  Position mCenter;
  double mRadius;
} CollisionCirc;


// TODO: work around this with polymorphism
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

void resolveCollsion(PhysicsObject* tObject, CollisionRect tObjectRect, CollisionRect tOtherRect);
int checkCollision(CollisionRect tRect1, CollisionRect tRect2);
int checkCollisionCirc(CollisionCirc tCirc1, CollisionCirc tCirc2);

CollisionObjectRect makeCollisionObjectRect(Position tTopLeft, Position tBottomRight, PhysicsObject* tPhysics);
CollisionObjectCirc makeCollisionObjectCirc(Position tCenter, double tRadius, PhysicsObject* tPhysics);

int checkCollisionObjectCirc(CollisionObjectCirc tObj1, CollisionObjectCirc tObj2);


#endif
