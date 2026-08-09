#pragma once

#include <prism/actorhandler.h>
#include <prism/collision.h>

namespace prism {
	ActorBlueprint getBlitzPlatformingHandler();

	void addBlitzPlatformingPlayerComponent(int tEntityID, const CollisionRect& tCollisionRect);
	void addBlitzPlatformingPlayerJump(int tEntityID);
	void addBlitzPlatformingPlayerMovement(int tEntityID, float t); // [-1, 1]
	bool isBlitzPlatformingPlayerJumping(int tEntityID);
	void setBlitzPlatformingPlayerMovementXStopped(int tEntityID);

	void setBlitzPlatformClampedToScreen(bool tIsClamped);
	void setBlitzPlatformGravity(float tGravity);

	void addBlitzPlatformingSolidTileComponent(int tEntityID, const CollisionRect& tCollisionRect);

	void removeBlitzPlatformingComponent(int tEntityID);

	void setBlitzPlatformingPausing(bool tIsPaused);
}