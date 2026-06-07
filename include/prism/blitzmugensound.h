#pragma once

#include "mugensoundfilereader.h"
#include "actorhandler.h"

namespace prism {

ActorBlueprint getBlitzMugenSoundHandler();

void addBlitzMugenSoundComponent(int tEntityID, MugenSounds* tSounds);
void playEntityMugenSound(int tEntityID, int tGroup, int tSample);

void imguiBlitzMugenSoundHandler();

}