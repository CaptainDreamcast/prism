#pragma once

#include "mugensoundfilereader.h"
#include "actorhandler.h"

extern ActorBlueprint BlitzMugenSoundHandler;

void addBlitzMugenSoundComponent(int tEntityID, MugenSounds* tSounds);
void playEntityMugenSound(int tEntityID, int tGroup, int tSample);