#include "prism/screeneffect.h"

#include <GL/glew.h>

namespace prism {

	extern SDL_Renderer* gRenderer;

	void setScreenColor(Color tColor) {
		float r, g, b;
		getRGBFromColor(tColor, &r, &g, &b);
		glClearColor((GLclampf)r, (GLclampf)g, (GLclampf)b, (GLclampf)1);

		disableDrawing();
	}

	void setScreenBackgroundColorRGB(float tR, float tG, float tB)
	{
		glClearColor((GLclampf)tR, (GLclampf)tG, (GLclampf)tB, (GLclampf)1);
	}

	void unsetScreenColor() {
		enableDrawing();
	}

}