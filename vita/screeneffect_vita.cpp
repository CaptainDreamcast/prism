#include "prism/screeneffect.h"

namespace prism {

	void setScreenColor(Color tColor) {
		double r, g, b;
		getRGBFromColor(tColor, &r, &g, &b);
		vita2d_set_clear_color(RGBA8(uint8_t(0xFF * r), uint8_t(0xFF * g), uint8_t(0xFF * b), 0xFF));

		disableDrawing();
	}

	void setScreenBackgroundColorRGB(double tR, double tG, double tB)
	{
		vita2d_set_clear_color(RGBA8(uint8_t(0xFF * tR), uint8_t(0xFF * tG), uint8_t(0xFF * tB), 0xFF));
	}

	void unsetScreenColor() {
		enableDrawing();
	}

}