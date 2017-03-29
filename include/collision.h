#ifndef TARI_COLLISION
#define TARI_COLLISION

#include "physics.h"
#include "geometry.h"

typedef GeoRectangle CollisionRect;
typedef Circle CollisionCirc;

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

typedef struct {
	CollisionType mType;
	void* mData; 
	Position* mBasePosition;
} Collider;

fup void resolveCollsion(PhysicsObject* tObject, CollisionRect tObjectRect, CollisionRect tOtherRect);
fup int checkCollision(CollisionRect tRect1, CollisionRect tRect2);
fup int checkCollisionCirc(CollisionCirc tCirc1, CollisionCirc tCirc2);
fup int checkCollisionCircRect(CollisionCirc tCirc1, CollisionRect tCirc2);
fup int checkCollisionCollider(Collider tCollider1, Collider tCollider2);

fup CollisionObjectRect makeCollisionObjectRect(Position tTopLeft, Position tBottomRight, PhysicsObject* tPhysics);
fup CollisionObjectCirc makeCollisionObjectCirc(Position tCenter, double tRadius, PhysicsObject* tPhysics);
fup CollisionRect adjustCollisionObjectRect(CollisionObjectRect* tObj);
fup CollisionCirc adjustCollisionObjectCirc(CollisionObjectCirc* tObj);


fup CollisionRect makeCollisionRect(Position tTopLeft, Position tBottomRight);
fup CollisionCirc makeCollisionCirc(Position tCenter, double tRadius);

fup int checkCollisionObjectCirc(CollisionObjectCirc tObj1, CollisionObjectCirc tObj2);
fup int checkCollisionObjectRect(CollisionObjectRect tObj1, CollisionObjectRect tObj2);
fup int checkCollisionObjectCircRect(CollisionObjectCirc tObj1, CollisionObjectRect tObj2);

fup Collider makeColliderFromRect(CollisionRect tRect);
fup void setColliderBasePosition(Collider* tCollider, Position* tBasePosition);
fup void destroyCollider(Collider* tCollider);



#endif
