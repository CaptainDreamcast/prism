#include "prism/screeneffect.h"

#include "prism/drawing.h"
#include "prism/system.h"
#include "prism/log.h"

static struct {
	int mIsScreenColored;

} gData;

void setScreenColor(Color tColor) {
	if(gData.mIsScreenColored) return;

	double r, g, b;
	getRGBFromColor(tColor, &r, &g, &b);

	setScreenBackgroundColorRGB(r, g, b);

	disableDrawing();
	gData.mIsScreenColored = 1;
}

void setScreenBackgroundColorRGB(double tR, double tG, double tB) {
	pvr_set_bg_color(tR, tG, tB);
}

void unsetScreenColor() {
	if(!gData.mIsScreenColored) return;
	enableDrawing();
	gData.mIsScreenColored = 0;
}
