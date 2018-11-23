#pragma once

#include <prism/actorhandler.h>
#include <prism/animation.h>

#define PARTICLE_ANGLE_UP ((M_PI / 2) * 3)

ActorBlueprint getBlitzParticleHandler();

void addBlitzParticles(int tAmount, Position tPosition, Position tPositionRange, double tSpeed, double tSpeedRange, double tAngle, double tAngleRange, Velocity tGravity, Vector3D tColor, Vector3D tColorRange, Duration tLifetime, Duration tLifetimeRange);
void addBlitzParticle(Position tPosition, Position tPositionRange, double tSpeed, double tSpeedRange, double tAngle, double tAngleRange, Velocity tGravity, Vector3D tColor, Vector3D tColorRange, Duration tLifetime, Duration tLifetimeRange);
