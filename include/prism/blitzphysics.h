#pragma once

#include "blitzcomponent.h"
#include "geometry.h"
#include "actorhandler.h"
#include "physics.h"

ActorBlueprint getBlitzPhysicsHandler();

void addBlitzPhysicsComponent(int tEntityID);
void setBlitzPhysicsGravity(int tEntityID, Acceleration tGravity);
void addBlitzPhysicsImpulse(int tEntityID, Acceleration tImpulse);
void setBlitzPhysicsDragFactorOnCollision(int tEntityID, Vector3D tDragFactor);

Velocity getBlitzPhysicsVelocity(int tEntityID);
Velocity* getBlitzPhysicsVelocityReference(int tEntityID);
void setBlitzPhysicsVelocity(int tEntityID, Velocity tVelocity);
void setBlitzPhysicsVelocityX(int tEntityID, double tX);
void addBlitzPhysicsVelocityX(int tEntityID, double tX);
double getBlitzPhysicsVelocityY(int tEntityID);
void setBlitzPhysicsVelocityY(int tEntityID, double tY);
void addBlitzPhysicsVelocityY(int tEntityID, double tY);
