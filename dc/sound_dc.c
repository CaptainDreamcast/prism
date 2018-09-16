#include "prism/sound.h"

#include <kos.h>
#include <dc/sound/sound.h>

#include "prism/file.h"
#include "prism/thread.h"

static struct {

	int mVolume;
	double mPanning;

	int mIsStreamingSoundFile;
	FileHandler mSoundFile;
	snd_stream_hnd_t mSoundStream;
	
	int mSampleAmount;
	int mLastSoundBufferRequested;
	short mSoundBuffer16[88000];		
	char mSoundBuffer[88000];	
	
} gData;

void initSound() {
	gData.mVolume = 1;
	gData.mPanning = 0;
	snd_init();
}

void shutdownSound(){
	
}

double getVolume() {
	return gData.mVolume;
}

void setVolume(double tVolume) {
	gData.mVolume = (int)(15*tVolume);
	spu_cdda_volume(gData.mVolume, gData.mVolume);
}

double getPanningValue() {
	return gData.mPanning;
}


void playTrack(int tTrack) {
	 cdrom_cdda_play(tTrack, tTrack, 15, CDDA_TRACKS);
}

void playTrackOnce(int tTrack) {
	 cdrom_cdda_play(tTrack, tTrack, 0, CDDA_TRACKS);
}

void stopTrack() {
	cdrom_cdda_pause();
}

void pauseTrack() {
	cdrom_cdda_pause();
}

void resumeTrack() {
	cdrom_cdda_resume();
}

#define STREAM_MUSIC_SAFETY_BUFFER 4000

static void* streamMusicFileCB(snd_stream_hnd_t tHandler, int tAmount, int * tActualAmount) {
	(void)tHandler;	


	int bufferedSamples = gData.mSampleAmount - gData.mLastSoundBufferRequested;
	int samplesNecessary = tAmount + STREAM_MUSIC_SAFETY_BUFFER - bufferedSamples;

	int requestedSampleSize = samplesNecessary*2;
	//printf("req %d\n", requestedSampleSize);
	size_t readSize = fileRead(gData.mSoundFile, gData.mSoundBuffer16, requestedSampleSize);
	int readSampleAmount = readSize/2;	

	if(gData.mLastSoundBufferRequested > 0) {
		gData.mSampleAmount -= gData.mLastSoundBufferRequested;
		memcpy(gData.mSoundBuffer, gData.mSoundBuffer + gData.mLastSoundBufferRequested, gData.mSampleAmount);
	}
	gData.mSampleAmount += readSampleAmount;

	int i;
	for(i = bufferedSamples; i < bufferedSamples+readSampleAmount; i++) {
		gData.mSoundBuffer[i] = (gData.mSoundBuffer16[i-bufferedSamples] >> 8);
	}

	//printf("read %d\n", readSize);
	

	*tActualAmount = tAmount;
	gData.mLastSoundBufferRequested = tAmount;

	return gData.mSoundBuffer;
}

static void* streamMusicFileCB2(snd_stream_hnd_t tHandler, int tAmount, int * tActualAmount) {
	(void)tHandler;	

	int samplesNecessary = tAmount;

	int requestedSampleSize = samplesNecessary;
	//printf("req %d\n", requestedSampleSize);
	size_t readSize = fileRead(gData.mSoundFile, gData.mSoundBuffer, requestedSampleSize);
	int readSampleAmount = readSize;	

	*tActualAmount = tAmount;

	return gData.mSoundBuffer;
}

static void streamMusicThread(void* tCaller) {
	(void)tCaller;

	snd_stream_start(gData.mSoundStream, 22050 / 2, 0);

	while(gData.mIsStreamingSoundFile) {
		if(snd_stream_poll(gData.mSoundStream) < 0) {
			// TODO: release file
			gData.mIsStreamingSoundFile = 0;
		} else {
			thd_sleep(10);
		}
	}

	
}

void streamMusicFile(char* tPath) {

	gData.mSoundFile = fileOpen(tPath, O_RDONLY);
	fileSeek(gData.mSoundFile, 1708, SEEK_SET);
	gData.mSampleAmount = 0;
	gData.mLastSoundBufferRequested = 0;

	gData.mSoundStream = snd_stream_alloc(streamMusicFileCB2, SND_STREAM_BUFFER_MAX);
	gData.mIsStreamingSoundFile = 1;

	startThread(streamMusicThread, NULL);
}

void streamMusicFileOnce(char* tPath) {
	(void)tPath;
	// TODO
}

void stopStreamingMusicFile() {
	
}

static ActorBlueprint MicrophoneHandler; // TODO: implement microphone

ActorBlueprint getMicrophoneHandlerActorBlueprint()
{
	return MicrophoneHandler;
}

double getMicrophoneVolume()
{
	return 0; // TODO: implement microphone
}
