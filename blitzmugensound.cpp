#include "prism/blitzmugensound.h"

#include "prism/datastructures.h"
#include "prism/blitzentity.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"

typedef struct {
	int mEntityID;
	MugenSounds* mSounds;
} SoundEntry;

static struct {
	IntMap mEntries;
} gBlitzMugenSoundData;

static void loadBlitzMugenSoundHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gBlitzMugenSoundData.mEntries = new_int_map();
}

static void unregisterEntity(int tEntityID);

static BlitzComponent getBlitzMugenSoundComponent() {
	return makeBlitzComponent(unregisterEntity);
}

ActorBlueprint getBlitzMugenSoundHandler() {
	return makeActorBlueprint(loadBlitzMugenSoundHandler);
}

void addBlitzMugenSoundComponent(int tEntityID, MugenSounds* tSounds)
{
	SoundEntry* e = (SoundEntry*)allocMemory(sizeof(SoundEntry));
	e->mEntityID = tEntityID;
	e->mSounds = tSounds;

	registerBlitzComponent(tEntityID, getBlitzMugenSoundComponent());
	int_map_push_owned(&gBlitzMugenSoundData.mEntries, tEntityID, e);
}

static SoundEntry* getSoundEntry(int tEntityID) {
	if (!int_map_contains(&gBlitzMugenSoundData.mEntries, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have a mugen sound component.", tEntityID);
		recoverFromError();
	}

	return (SoundEntry*)int_map_get(&gBlitzMugenSoundData.mEntries, tEntityID);
}

void playEntityMugenSound(int tEntityID, int tGroup, int tSample)
{
	SoundEntry* e = getSoundEntry(tEntityID);
	playMugenSound(e->mSounds, tGroup, tSample);
}

static void unregisterEntity(int tEntityID) {
	int_map_remove(&gBlitzMugenSoundData.mEntries, tEntityID);
}