#include "prism/pvr.h"

#include <kos.h>

#include <prism/debug.h>

void initiatePVR() {
	pvr_init_params_t params = {
        /* Enable opaque and translucent polygons with size 16 */
        { PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0 },

        /* Vertex buffer size 192K */
        192 * 1024,

        /* No DMA */
        0,

        /* No FSAA */
        0,

        /* Translucent Autosort enabled. */
        0
    };

    pvr_init(&params);
}

void drawPVRDebuggingBorder(double r, double g, double b) {
    if(!isInDevelopMode()) return;

    vid_border_color((int)(r*255), (int)(g*255), (int)(b*255));
}
