#include "tari/screeneffect.h"

#include "tari/drawing.h"
#include "tari/system.h"
#include "tari/log.h"

static struct {
	int mIsScreenColored;

} gData;

void setScreenColor(Color tColor) {
	if(gData.mIsScreenColored) return;

	double r, g, b;
	getRGBFromColor(tColor, &r, &g, &b);

	pvr_set_bg_color(r, g, b);
	disableDrawing();
	gData.mIsScreenColored = 1;
}

void unsetScreenColor() {
	if(!gData.mIsScreenColored) return;
	enableDrawing();
	gData.mIsScreenColored = 0;
}
