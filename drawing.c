#include "include/drawing.h"

#include "include/log.h"

void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB) {
	if (tColor == COLOR_BLACK) {
		(*tR) = (*tG) = (*tB) = 0;
	}
	else if (tColor == COLOR_RED) {
		(*tR) = 1.0f;
		(*tG) = (*tB) = 0;
	}
	else if (tColor == COLOR_GREEN) {
		(*tG) = 1.0f;
		(*tR) = (*tB) = 0;
	}
	else if (tColor == COLOR_BLUE) {
		(*tB) = 1.0f;
		(*tG) = (*tR) = 0;
	}
	else if (tColor == COLOR_YELLOW) {
		(*tG) = (*tR) = 1.0f;
		(*tB) = 0;
	}
	else {
		(*tR) = (*tG) = (*tB) = 1.0f;
	}
}

Rectangle makeRectangleFromTexture(TextureData tTexture) {
	return makeRectangle(0, 0, tTexture.mTextureSize.x - 1, tTexture.mTextureSize.y - 1);
}

Position getTextureMiddlePosition(TextureData tTexture) {
	return makePosition(tTexture.mTextureSize.x / 2, tTexture.mTextureSize.y / 2, 0);
}

Rectangle makeRectangle(int x, int y, int w, int h) {
	Rectangle ret;
	ret.topLeft.x = x;
	ret.topLeft.y = y;
	ret.bottomRight.x = x + w;
	ret.bottomRight.y = y + h;
	return ret;
}

void printRectangle(Rectangle r) {
	logg("Rectangle");
	logDouble(r.topLeft.x);
	logDouble(r.topLeft.y);
	logDouble(r.bottomRight.x);
	logDouble(r.bottomRight.y);

}
