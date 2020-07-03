#pragma once

#include <prism/actorhandler.h>
#include <prism/animation.h>

#define PARTICLE_ANGLE_UP ((M_PI / 2) * 3)

ActorBlueprint getBlitzParticleHandler();

void addBlitzParticles(int tAmount, const Position& tPosition, const Position& tPositionRange, double tSpeed, double tSpeedRange, double tAngle, double tAngleRange, const Velocity& tGravity, const Vector3D& tColor, const Vector3D& tColorRange, Duration tLifetime, Duration tLifetimeRange);
void addBlitzParticle(const Position& tPosition, const Position& tPositionRange, double tSpeed, double tSpeedRange, double tAngle, double tAngleRange, const Velocity& tGravity, const Vector3D& tColor, const Vector3D& tColorRange, Duration tLifetime, Duration tLifetimeRange);
