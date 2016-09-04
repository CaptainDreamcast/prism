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

void resolveCollsion(PhysicsObject* tObject, CollisionRect tObjectRect, CollisionRect tOtherRect);
int checkCollision(CollisionRect tRect1, CollisionRect tRect2);

#endif
