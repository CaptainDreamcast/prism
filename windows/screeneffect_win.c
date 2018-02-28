#include "prism/screeneffect.h"

#include <GL/glew.h>

extern SDL_Renderer* gRenderer;

void setScreenColor(Color tColor) {
	double r, g, b;
	getRGBFromColor(tColor, &r, &g, &b);
	glClearColor((GLclampf)r, (GLclampf)g, (GLclampf)b, (GLclampf)1);

	disableDrawing();
}

void setScreenBackgroundColorRGB(double tR, double tG, double tB)
{
	glClearColor((GLclampf)tR, (GLclampf)tG, (GLclampf)tB, (GLclampf)1);
}

void unsetScreenColor() {
	enableDrawing();
}