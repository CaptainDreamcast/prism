#pragma once

#include "mugensoundfilereader.h"
#include "actorhandler.h"

ActorBlueprint getBlitzMugenSoundHandler();

void addBlitzMugenSoundComponent(int tEntityID, MugenSounds* tSounds);
void playEntityMugenSound(int tEntityID, int tGroup, int tSample);