#pragma once

#include <prism/actorhandler.h>
#include <prism/mugenspritefilereader.h>
#include <prism/mugenanimationreader.h>

namespace prism {

ActorBlueprint getLifeBarHandler();

enum class LifeBarType {
    STRETCH,
    WIDTH,
};
int addLifeBar(const Vector3D& tPosition, MugenSpriteFile& mSprites, MugenAnimations& mAnimations, int tFGAnimNo, int tBGAnimNo, LifeBarType tType = LifeBarType::STRETCH, int tStartValue = 100, int tMaxValue = 100, int tFullSize = 256, const Vector3D& tFGOffset = Vector3D(0, 0, 0));
void removeLifeBar(int tID);

int getLifeBarPercentage(int tID);
void setLifeBarPercentage(int tID, double tPercentage);
int getLifeBarValue(int tID);
void setLifeBarValue(int tID, int tValue);
void setLifeBarVisibility(int tID, int tIsVisible);

void removeAllLifebars();

}