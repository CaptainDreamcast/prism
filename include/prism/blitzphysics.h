#pragma once

#include "blitzcomponent.h"
#include "geometry.h"
#include "actorhandler.h"
#include "physics.h"

namespace prism {

ActorBlueprint getBlitzPhysicsHandler();

void addBlitzPhysicsComponent(int tEntityID);
void setBlitzPhysicsGravity(int tEntityID, const Acceleration& tGravity);
void addBlitzPhysicsImpulse(int tEntityID, const Acceleration& tImpulse);
void setBlitzPhysicsDragFactorOnCollision(int tEntityID, const Vector3D& tDragFactor);

Velocity getBlitzPhysicsVelocity(int tEntityID);
Velocity* getBlitzPhysicsVelocityReference(int tEntityID);
void setBlitzPhysicsVelocity(int tEntityID, const Velocity& tVelocity);
void setBlitzPhysicsVelocityX(int tEntityID, float tX);
void addBlitzPhysicsVelocity(int tEntityID, const Velocity& tVelocity);
void addBlitzPhysicsVelocityX(int tEntityID, float tX);
float getBlitzPhysicsVelocityY(int tEntityID);
void setBlitzPhysicsVelocityY(int tEntityID, float tY);
void addBlitzPhysicsVelocityY(int tEntityID, float tY);

void imguiBlitzPhysicsHandler();

}