#include "prism/sound.h"

#include <kos.h>
#include <dc/sound/sound.h>
#include <vorbis/sndoggvorbis.h>

#include "prism/file.h"
#include "prism/thread.h"

#define BUF_SIZE 65536			/* Size of buffer */


static struct {

	int mVolume;
	double mPanning;

	int mIsStreamingSoundFile;
	
	
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


void streamMusicFile(char* tPath) {

	char fullPath[1024];
	getFullPath(fullPath, tPath);	
	sndoggvorbis_start(fullPath, 0);
	

	gData.mIsStreamingSoundFile = 1;
}

void streamMusicFileOnce(char* tPath) {
	(void)tPath;
	// TODO
}

void stopStreamingMusicFile() {
	if(!gData.mIsStreamingSoundFile) return;
	sndoggvorbis_stop();
	gData.mIsStreamingSoundFile = 0;
}

uint64_t getStreamingSoundTimeElapsedInMilliseconds() {
	return sndoggvorbis_getposition();
}

int isPlayingStreamingMusic() {
	return gData.mIsStreamingSoundFile;
}

void stopMusic() {
	stopTrack();
	stopStreamingMusicFile();
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
