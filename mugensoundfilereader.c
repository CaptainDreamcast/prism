#include "prism/mugensoundfilereader.h"

#include <assert.h>

#include <prism/file.h>
#include <prism/memoryhandler.h>
#include <prism/soundeffect.h>
#include <prism/log.h>
#include <prism/system.h>

typedef struct {
	uint32_t mNextFileOffset;
	uint32_t mSubfileLength;

	uint32_t mGroupNumber;
	uint32_t mSampleNumber;
} MugenSoundFileSubFileHeader;

typedef struct {
	char mSignature[12];
	uint16_t mVersionLow;
	uint16_t mVersionHigh;

	uint32_t mNumberOfSounds;
	uint32_t mFirstSubFileOffset;

	char mComments[488];
} MugenSoundFileHeader;

static void addSoundGroup(MugenSounds* tSounds, int tGroupNumber) {
	MugenSoundGroup* e = allocMemory(sizeof(MugenSoundGroup));
	e->mSamples = new_int_map();
	int_map_push_owned(&tSounds->mGroups, tGroupNumber, e);
}

static void loadSingleMugenSoundSubfile(MugenSounds* tSounds, BufferPointer* p, Buffer b) {
	MugenSoundFileSubFileHeader subHeader;

	readFromBufferPointer(&subHeader, p, sizeof(MugenSoundFileSubFileHeader));

	MugenSoundGroup* group;
	if (!int_map_contains(&tSounds->mGroups, subHeader.mGroupNumber)) {
		addSoundGroup(tSounds, subHeader.mGroupNumber);
	}
	group = int_map_get(&tSounds->mGroups, subHeader.mGroupNumber);

	char* waveFileData = allocMemory(subHeader.mSubfileLength);
	readFromBufferPointer(waveFileData, p, subHeader.mSubfileLength);
	Buffer waveFileBuffer = makeBufferOwned(waveFileData, subHeader.mSubfileLength);

	MugenSoundSample* e = allocMemory(sizeof(MugenSoundSample));
	e->mSoundEffectID = loadSoundEffectFromBuffer(waveFileBuffer);
	freeBuffer(waveFileBuffer);

	int_map_push_owned(&group->mSamples, subHeader.mSampleNumber, e);

	if (subHeader.mNextFileOffset != 0) {
		(*p) = (BufferPointer)(((uint32_t)b.mData) + subHeader.mNextFileOffset);
	}
}

static MugenSounds loadMugenSoundFileFromBuffer(Buffer b) {
	MugenSounds ret;
	ret.mGroups = new_int_map();

	BufferPointer p = getBufferPointer(b);

	MugenSoundFileHeader header;
	readFromBufferPointer(&header, &p, sizeof(MugenSoundFileHeader));

	p = (BufferPointer)(((uint32_t)b.mData) + header.mFirstSubFileOffset);
	int i;
	for (i = 0; i < (int)header.mNumberOfSounds; i++) {
		loadSingleMugenSoundSubfile(&ret, &p, b);
	}

	return ret;
}

MugenSounds loadMugenSoundFile(char * tPath)
{
	Buffer b = fileToBuffer(tPath);
	MugenSounds ret = loadMugenSoundFileFromBuffer(b);
	freeBuffer(b);

	return ret;
}

MugenSounds createEmptyMugenSoundFile() {
	MugenSounds ret;
	ret.mGroups = new_int_map();
	return ret;
}

int playMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	assert(int_map_contains(&tSounds->mGroups, tGroup));
	MugenSoundGroup* group = int_map_get(&tSounds->mGroups, tGroup);

	assert(int_map_contains(&group->mSamples, tSample));
	MugenSoundSample* sample = int_map_get(&group->mSamples, tSample);

	return playSoundEffect(sample->mSoundEffectID);
}

int tryPlayMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	if (!hasMugenSound(tSounds, tGroup, tSample)) {
		logWarningFormat("Unable to find sound %d %d.", tGroup, tSample);
		return 0;
	}

	playMugenSound(tSounds, tGroup, tSample);
	return 1;
}

int hasMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	if(!int_map_contains(&tSounds->mGroups, tGroup)) return 0;
	MugenSoundGroup* group = int_map_get(&tSounds->mGroups, tGroup);

	return int_map_contains(&group->mSamples, tSample);
}
