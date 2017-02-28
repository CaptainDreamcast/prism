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

void resolveCollsion(PhysicsObject* tObject, CollisionRect tObjectRect, CollisionRect tOtherRect);
int checkCollision(CollisionRect tRect1, CollisionRect tRect2);
int checkCollisionCirc(CollisionCirc tCirc1, CollisionCirc tCirc2);
int checkCollisionCircRect(CollisionCirc tCirc1, CollisionRect tCirc2);
int checkCollisionCollider(Collider tCollider1, Collider tCollider2);

CollisionObjectRect makeCollisionObjectRect(Position tTopLeft, Position tBottomRight, PhysicsObject* tPhysics);
CollisionObjectCirc makeCollisionObjectCirc(Position tCenter, double tRadius, PhysicsObject* tPhysics);
CollisionRect adjustCollisionObjectRect(CollisionObjectRect* tObj);
CollisionCirc adjustCollisionObjectCirc(CollisionObjectCirc* tObj);


CollisionRect makeCollisionRect(Position tTopLeft, Position tBottomRight);
CollisionCirc makeCollisionCirc(Position tCenter, double tRadius);

int checkCollisionObjectCirc(CollisionObjectCirc tObj1, CollisionObjectCirc tObj2);
int checkCollisionObjectRect(CollisionObjectRect tObj1, CollisionObjectRect tObj2);
int checkCollisionObjectCircRect(CollisionObjectCirc tObj1, CollisionObjectRect tObj2);

Collider makeColliderFromRect(CollisionRect tRect);
void setColliderBasePosition(Collider* tCollider, Position* tBasePosition);
void destroyCollider(Collider* tCollider);



#endif
