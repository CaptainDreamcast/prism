#include "prism/pvr.h"

#include <kos.h>

#include <prism/debug.h>

void initiatePVR() {
	pvr_init_defaults();
}

void drawPVRDebuggingBorder(double r, double g, double b) {
    if(!isInDevelopMode()) return;

    vid_border_color((int)(r*255), (int)(g*255), (int)(b*255));
}
