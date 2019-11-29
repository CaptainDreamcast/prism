#include "prism/mugensoundfilereader.h"

#include <assert.h>

#include <prism/file.h>
#include <prism/filereader.h>
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

static struct {
	FileReader mFileReader;
} gMugenSoundFileReaderData;

static void addSoundGroup(MugenSounds* tSounds, int tGroupNumber) {
	MugenSoundGroup* e = (MugenSoundGroup*)allocMemory(sizeof(MugenSoundGroup));
	e->mSamples = new_int_map();
	int_map_push_owned(&tSounds->mGroups, tGroupNumber, e);
}

static void loadSingleMugenSoundSubfile(MugenSounds* tSounds) {
	MugenSoundFileSubFileHeader subHeader;
	gMugenSoundFileReaderData.mFileReader.mRead(&gMugenSoundFileReaderData.mFileReader, &subHeader, sizeof(MugenSoundFileSubFileHeader));

	MugenSoundGroup* group;
	if (!int_map_contains(&tSounds->mGroups, subHeader.mGroupNumber)) {
		addSoundGroup(tSounds, subHeader.mGroupNumber);
	}
	group = (MugenSoundGroup*)int_map_get(&tSounds->mGroups, subHeader.mGroupNumber);

	Buffer waveFileBuffer = gMugenSoundFileReaderData.mFileReader.mReadBufferReadOnly(&gMugenSoundFileReaderData.mFileReader, subHeader.mSubfileLength);

	MugenSoundSample* e = (MugenSoundSample*)allocMemory(sizeof(MugenSoundSample));
	e->mSoundEffectID = loadSoundEffectFromBuffer(waveFileBuffer);
	freeBuffer(waveFileBuffer);

	int_map_push_owned(&group->mSamples, subHeader.mSampleNumber, e);

	if (subHeader.mNextFileOffset != 0) {
		gMugenSoundFileReaderData.mFileReader.mSeek(&gMugenSoundFileReaderData.mFileReader, subHeader.mNextFileOffset);
	}
}

static MugenSounds loadMugenSoundFileWithReader() {
	MugenSounds ret;
	ret.mGroups = new_int_map();

	MugenSoundFileHeader header;
	gMugenSoundFileReaderData.mFileReader.mRead(&gMugenSoundFileReaderData.mFileReader, &header, sizeof(MugenSoundFileHeader));

	gMugenSoundFileReaderData.mFileReader.mSeek(&gMugenSoundFileReaderData.mFileReader, header.mFirstSubFileOffset);

	int i;
	for (i = 0; i < (int)header.mNumberOfSounds; i++) {
		loadSingleMugenSoundSubfile(&ret);
	}

	return ret;
}

static void initMugenSoundFileReader(uint32_t tFileSize) {
	if (tFileSize <= 1024 * 1024) {
		gMugenSoundFileReaderData.mFileReader = getBufferFileReader();
	}
	else {
		gMugenSoundFileReaderData.mFileReader = getFileFileReader();
	}
}

MugenSounds loadMugenSoundFile(const char * tPath)
{
	initMugenSoundFileReader(getFileSize(tPath));
	gMugenSoundFileReaderData.mFileReader.mInit(&gMugenSoundFileReaderData.mFileReader, tPath);
	MugenSounds ret = loadMugenSoundFileWithReader();
	gMugenSoundFileReaderData.mFileReader.mDelete(&gMugenSoundFileReaderData.mFileReader);

	return ret;
}

static int unloadMugenSoundFileSample(void* tCaller, void* tData) {
	(void)tCaller;
	MugenSoundSample* e = (MugenSoundSample*)tData;
	unloadSoundEffect(e->mSoundEffectID);
	return 1;
}

static int unloadMugenSoundFileGroup(void* tCaller, void* tData) {
	(void)tCaller;
	MugenSoundGroup* e = (MugenSoundGroup*)tData;
	
	int_map_remove_predicate(&e->mSamples, unloadMugenSoundFileSample, NULL);
	delete_int_map(&e->mSamples);

	return 1;
}

void unloadMugenSoundFile(MugenSounds * tSounds)
{
	int_map_remove_predicate(&tSounds->mGroups, unloadMugenSoundFileGroup, NULL);
	delete_int_map(&tSounds->mGroups);
}

MugenSounds createEmptyMugenSoundFile() {
	MugenSounds ret;
	ret.mGroups = new_int_map();
	return ret;
}

int playMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	assert(int_map_contains(&tSounds->mGroups, tGroup));
	MugenSoundGroup* group = (MugenSoundGroup*)int_map_get(&tSounds->mGroups, tGroup);

	assert(int_map_contains(&group->mSamples, tSample));
	MugenSoundSample* sample = (MugenSoundSample*)int_map_get(&group->mSamples, tSample);

	return playSoundEffect(sample->mSoundEffectID);
}

int tryPlayMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	if (!hasMugenSound(tSounds, tGroup, tSample)) {
		return 0;
	}

	playMugenSound(tSounds, tGroup, tSample);
	return 1;
}

int hasMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	if(!int_map_contains(&tSounds->mGroups, tGroup)) return 0;
	MugenSoundGroup* group = (MugenSoundGroup*)int_map_get(&tSounds->mGroups, tGroup);

	return int_map_contains(&group->mSamples, tSample);
}
