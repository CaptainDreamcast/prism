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

static int loadSingleMugenSoundSubfileAndReturnIfOver(MugenSounds* tSounds) {
	MugenSoundFileSubFileHeader subHeader;
	gMugenSoundFileReaderData.mFileReader.mRead(&gMugenSoundFileReaderData.mFileReader, &subHeader, sizeof(MugenSoundFileSubFileHeader));

	MugenSoundGroup* group;
	if (!int_map_contains(&tSounds->mGroups, subHeader.mGroupNumber)) {
		addSoundGroup(tSounds, subHeader.mGroupNumber);
	}
	group = (MugenSoundGroup*)int_map_get(&tSounds->mGroups, subHeader.mGroupNumber);
	MugenSoundSample* e = (MugenSoundSample*)allocMemory(sizeof(MugenSoundSample));
	if (unsigned(getAvailableSoundMemory()) >= subHeader.mSubfileLength * 2) {
		Buffer waveFileBuffer = gMugenSoundFileReaderData.mFileReader.mReadBufferReadOnly(&gMugenSoundFileReaderData.mFileReader, subHeader.mSubfileLength);
		e->mSoundEffectID = loadSoundEffectFromBuffer(waveFileBuffer);
		freeBuffer(waveFileBuffer);
	}
	else {
		logWarningFormat("Early out for sound effect %d %d, does not fit in sound memory.", subHeader.mGroupNumber, subHeader.mSampleNumber);
		e->mSoundEffectID = -1;
	}

	int_map_push_owned(&group->mSamples, subHeader.mSampleNumber, e);

	if (e->mSoundEffectID == -1) {
		return 1;
	}

	if (subHeader.mNextFileOffset != 0) {
		gMugenSoundFileReaderData.mFileReader.mSeek(&gMugenSoundFileReaderData.mFileReader, subHeader.mNextFileOffset);
	}
	return 0;
}

static MugenSounds loadMugenSoundFileWithReader() {
	MugenSounds ret;
	ret.mGroups = new_int_map();

	MugenSoundFileHeader header;
	gMugenSoundFileReaderData.mFileReader.mRead(&gMugenSoundFileReaderData.mFileReader, &header, sizeof(MugenSoundFileHeader));

	gMugenSoundFileReaderData.mFileReader.mSeek(&gMugenSoundFileReaderData.mFileReader, header.mFirstSubFileOffset);

	int i;
	for (i = 0; i < (int)header.mNumberOfSounds; i++) {
		if (loadSingleMugenSoundSubfileAndReturnIfOver(&ret)) {
			break;
		}
	}

	return ret;
}

static void initMugenSoundFileReader() {
	gMugenSoundFileReaderData.mFileReader = getFileFileReader();
}

MugenSounds loadMugenSoundFile(const char * tPath)
{
	initMugenSoundFileReader();
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

static MugenSoundSample* getMugenSoundSample(MugenSounds* tSounds, int tGroup, int tSample) {
	assert(int_map_contains(&tSounds->mGroups, tGroup));
	MugenSoundGroup* group = (MugenSoundGroup*)int_map_get(&tSounds->mGroups, tGroup);

	assert(int_map_contains(&group->mSamples, tSample));
	return (MugenSoundSample*)int_map_get(&group->mSamples, tSample);
}

int playMugenSound(MugenSounds* tSounds, int tGroup, int tSample)
{
	const auto sample = getMugenSoundSample(tSounds, tGroup, tSample);
	return playSoundEffect(sample->mSoundEffectID);
}

int playMugenSoundAdvanced(MugenSounds* tSounds, int tGroup, int tSample, double tVolume, int tChannel, double tFrequencyMultiplier, int tIsLooping, double tPanning)
{
	const auto sample = getMugenSoundSample(tSounds, tGroup, tSample);
	int channel = playSoundEffectChannel(sample->mSoundEffectID, tChannel, tVolume, tFrequencyMultiplier, tIsLooping);
	panSoundEffect(channel, tPanning);
	return channel;
}

int tryPlayMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	if (!hasMugenSound(tSounds, tGroup, tSample)) {
		return 0;
	}

	playMugenSound(tSounds, tGroup, tSample);
	return 1;
}

int tryPlayMugenSoundAdvanced(MugenSounds* tSounds, int tGroup, int tSample, double tVolume, int tChannel, double tFrequencyMultiplier, int tIsLooping, double tPanning)
{
	if (!hasMugenSound(tSounds, tGroup, tSample)) {
		return 0;
	}

	playMugenSoundAdvanced(tSounds, tGroup, tSample, tVolume, tChannel, tFrequencyMultiplier, tIsLooping, tPanning);
	return 1;
}

int hasMugenSound(MugenSounds * tSounds, int tGroup, int tSample)
{
	if(!int_map_contains(&tSounds->mGroups, tGroup)) return 0;
	MugenSoundGroup* group = (MugenSoundGroup*)int_map_get(&tSounds->mGroups, tGroup);

	return int_map_contains(&group->mSamples, tSample);
}
