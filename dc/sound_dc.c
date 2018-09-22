#include "prism/sound.h"

#include <kos.h>
#include <dc/sound/sound.h>
#include <oggvorbis/sndoggvorbis.h>

#include "prism/file.h"
#include "prism/thread.h"

#define BUF_SIZE 65536			/* Size of buffer */


static struct {

	int mVolume;
	double mPanning;

	int mIsStreamingSoundFile;
	FileHandler mSoundFile;
	snd_stream_hnd_t mSoundStream;
	
	int mSampleAmount;
	int mLastSoundBufferRequested;
	uint8_t mSoundBuffer[(BUF_SIZE+16384)*2];	
	
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

	int samplesNecessary = tAmount;

	int requestedSampleSize = samplesNecessary * 2;
	printf("req %d\n", requestedSampleSize);
	size_t readSize = fileRead(gData.mSoundFile, gData.mSoundBuffer, requestedSampleSize);
	
	int i;
	for(i = 0; i < tAmount; i++) {
	  	gData.mSoundBuffer[i] = gData.mSoundBuffer[i*2];
	}

	*tActualAmount = tAmount;

	return gData.mSoundBuffer;
}

static void streamMusicThread(void* tCaller) {
	(void)tCaller;

	snd_stream_start(gData.mSoundStream, 44100, 0);

	while(gData.mIsStreamingSoundFile) {
		if(snd_stream_poll(gData.mSoundStream) < 0) {
			// TODO: release file
			gData.mIsStreamingSoundFile = 0;
		} else {
			thd_sleep(50);
		}
	}

	
}

void streamMusicFile(char* tPath) {

	gData.mSoundFile = fileOpen(tPath, O_RDONLY);
	fileSeek(gData.mSoundFile, 226, SEEK_SET);
	gData.mSampleAmount = 0;
	gData.mLastSoundBufferRequested = 0;

	gData.mSoundStream = snd_stream_alloc(streamMusicFileCB, SND_STREAM_BUFFER_MAX);
	gData.mIsStreamingSoundFile = 1;

	startThread(streamMusicThread, NULL);
}

void streamMusicFileOnce(char* tPath) {
	(void)tPath;
	// TODO
}

void stopStreamingMusicFile() {
	
}

uint64_t getStreamingSoundTimeElapsedInMilliseconds() {
	return 0;
}

int isPlayingStreamingMusic() {
	return gData.mIsStreamingSoundFile;
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
