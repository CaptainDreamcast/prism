#include "tari/sound.h"

#include <kos.h>
#include <dc/sound/sound.h>

static struct {

	double mVolume;
	double mPanning;

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
	gData.mVolume = tVolume;
}

double getPanningValue() {
	return gData.mPanning;
}


void playTrack(int tTrack) {
	return;
	 cdrom_cdda_play(tTrack, tTrack, 15, CDDA_TRACKS);
}

void stopTrack() {
	return;
	cdrom_cdda_pause();
}

void pauseTrack() {
	return;
	cdrom_cdda_pause();
}

void resumeTrack() {
	return;
	cdrom_cdda_resume();
}
