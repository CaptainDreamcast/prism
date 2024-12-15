#include "actorhandler.h"
#include "mugenspritefilereader.h"
#include "mugenanimationreader.h"
#include "mugensoundfilereader.h"

namespace prism {

int addPrismIntroText(const Position& tPosition, int tAnimation, const Vector2DI& tSFX, int tDuration, MugenSpriteFile* tSprites, MugenAnimations* tAnimations, MugenSounds* tSounds);
int hasPrismIntroTextFinished(int tID);

ActorBlueprint getPrismIntroTextHelper();

}