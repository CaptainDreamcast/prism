#include "geometry.h"
#include "mugenspritefilereader.h"
#include "mugenanimationreader.h"
#include "actorhandler.h"

int addPrismLifebar(int tEntityID, const Vector3D& tPosition, const Vector2D& tFGOffset, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, int tBGAnimation, int tFGAnimation, int tStartValue, int tMaxValue);

int getPrismLifebarValue(int tID);
void setPrismLifebarValue(int tID, int tValue);

ActorBlueprint getPrismLifebarHandler();