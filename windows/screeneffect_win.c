#include "tari/screeneffect.h"

extern SDL_Renderer* gRenderer;

void setScreenColor(Color tColor) {
	double rf, gf, bf;
	getRGBFromColor(tColor, &rf, &gf, &bf);
	uint8_t r, g, b;
	r = (uint8_t)(255 * rf);
	g = (uint8_t)(255 * gf);
	b = (uint8_t)(255 * bf);

	SDL_SetRenderDrawColor(gRenderer, r, g, b, 0xFF);

	disableDrawing();
}

void setScreenBackgroundColorRGB(double tR, double tG, double tB)
{
	uint8_t r, g, b;
	r = (uint8_t)(255 * tR);
	g = (uint8_t)(255 * tG);
	b = (uint8_t)(255 * tB);
	SDL_SetRenderDrawColor(gRenderer, r, g, b, 0xFF);
}

void unsetScreenColor() {
	enableDrawing();
}