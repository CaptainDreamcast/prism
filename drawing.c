#include "tari/drawing.h"

#include "tari/log.h"
#include "tari/math.h"

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

Rectangle scaleRectangle(Rectangle tRect, Vector3D tScale) {
	tRect.topLeft.x = (int)(tRect.topLeft.x*tScale.x);
	tRect.topLeft.y = (int)(tRect.topLeft.y*tScale.y);

	tRect.bottomRight.x = (int)(tRect.bottomRight.x*tScale.x);
	tRect.bottomRight.y = (int)(tRect.bottomRight.y*tScale.y);
	return tRect;
}

Rectangle translateRectangle(Rectangle tRect, Position tOffset) {
	tRect.topLeft.x += (int)tOffset.x;
	tRect.topLeft.y += (int)tOffset.y;

	tRect.bottomRight.x += (int)tOffset.x;
	tRect.bottomRight.y += (int)tOffset.y;
	return tRect;
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

void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor) {
	drawAdvancedText(tText, tPosition, makeFontSize(tSize, tSize), tColor, 0);
}

void drawAdvancedText(char* tText, Position tPosition, Vector3D tFontSize, Color tColor, TextSize tBreakSize) {
	drawMultilineText(tText, tPosition, tFontSize, tColor, makeFontSize(tBreakSize, 0), makePosition(INF, INF, INF));
}

Vector3D makeFontSize(int x, int y) {
	Vector3D ret;
	ret.x = x;
	ret.y = y;
	ret.z = 1;
	return ret;
}