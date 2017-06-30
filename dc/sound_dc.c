#include "tari/sound.h"

#include <kos.h>
#include <dc/sound/sound.h>

static struct {

	int mVolume;
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
	gData.mVolume = (int)(15*tVolume);
	spu_cdda_volume(gData.mVolume, gData.mVolume);
}

double getPanningValue() {
	return gData.mPanning;
}


void playTrack(int tTrack) {
	 cdrom_cdda_play(tTrack, tTrack, 15, CDDA_TRACKS);
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
