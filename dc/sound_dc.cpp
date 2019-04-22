#include "prism/sound.h"

#include <kos.h>
#include <dc/sound/sound.h>
#include <vorbis/sndoggvorbis.h>

#include "prism/file.h"
#include "prism/thread.h"
#include "prism/system.h"

#define BUF_SIZE 65536			/* Size of buffer */


static struct {

	int mVolume;
	double mPanning;

	int mIsStreamingSoundFile;
	
    uint64_t mStreamStartTime;
	
} gData;

void initSound() {
	gData.mVolume = 1;
	gData.mPanning = 0;
	snd_init();

	snd_stream_init();
    	sndoggvorbis_init();
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

static uint64_t getCurrentTimeInMilliseconds() {
    return getSystemTicks();
}

static void streamMusicFileGeneral(const char* tPath, int tLoop) {
	char fullPath[1024];
	getFullPath(fullPath, tPath);	
	sndoggvorbis_start(fullPath, tLoop);
	
    gData.mStreamStartTime = getCurrentTimeInMilliseconds();

	gData.mIsStreamingSoundFile = 1;
}

void streamMusicFile(const char* tPath) {
    streamMusicFileGeneral(tPath, 1);
}

void streamMusicFileOnce(const char* tPath) {
	streamMusicFileGeneral(tPath, 0);
}

void stopStreamingMusicFile() {
	if(!gData.mIsStreamingSoundFile) return;
	sndoggvorbis_stop();
	gData.mIsStreamingSoundFile = 0;
}

uint64_t getStreamingSoundTimeElapsedInMilliseconds() {
    if(!sndoggvorbis_isplaying()) {
        gData.mIsStreamingSoundFile = 0;
    }
    return (uint64_t)(getCurrentTimeInMilliseconds() - gData.mStreamStartTime);
}

int isPlayingStreamingMusic() {
	return gData.mIsStreamingSoundFile;
}

void stopMusic() {
	stopTrack();
	stopStreamingMusicFile();
}

void pauseMusic() {
    pauseTrack(); 
    // TODO: pause streaming
}

void resumeMusic() {
    resumeTrack(); 
    // TODO: resume streaming
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
