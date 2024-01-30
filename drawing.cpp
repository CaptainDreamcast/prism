#include "prism/drawing.h"

#include "prism/log.h"
#include "prism/math.h"

void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB) {

	switch (tColor) {
	case COLOR_BLACK:
		(*tR) = (*tG) = (*tB) = 0;
		break;
	case COLOR_RED:
		(*tR) = 1.0f;
		(*tG) = (*tB) = 0;
		break;
	case COLOR_DARK_RED:
		(*tR) = 0.5f;
		(*tG) = (*tB) = 0;
		break;
	case COLOR_GREEN:
		(*tG) = 1.0f;
		(*tR) = (*tB) = 0;
		break;
	case COLOR_DARK_GREEN:
		(*tG) = 0.5f;
		(*tR) = (*tB) = 0;
		break;
	case COLOR_BLUE:
		(*tB) = 1.0f;
		(*tG) = (*tR) = 0;
		break;
	case COLOR_DARK_BLUE:
		(*tB) = 0.5f;
		(*tG) = (*tR) = 0;
		break;
	case COLOR_YELLOW:
		(*tG) = (*tR) = 1.0f;
		(*tB) = 0;
		break;
	case COLOR_DARK_YELLOW:
		(*tG) = (*tR) = 0.5f;
		(*tB) = 0;
		break;
	case COLOR_CYAN:
		(*tR) = 0;
		(*tG) = (*tB) = 1.0f;
		break;
	case COLOR_MAGENTA:
		(*tR) = (*tB) = 1.0f;
		(*tG) = 0.0f;
		break;
	case COLOR_GRAY:
		(*tR) = (*tG) = (*tB) = 0.5f;
		break;
	case COLOR_LIGHT_GRAY:
		(*tR) = (*tG) = (*tB) = 0.75f;
		break;
	case COLOR_WHITE:
	default:
		(*tR) = (*tG) = (*tB) = 1.0f;
		break;
	}
}

int hasToLinebreak(const char* tText, int tCurrent, const Position& tTopLeft, const Position& tPos, const Vector3D& tFontSize, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize) {

	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;

	char word[1024];
	int positionsRead;
	int items = sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);
	if (items != 1) return 0;

	Position delta = Vector3D(positionsRead * tFontSize.x + (positionsRead - 1) * tBreakSize.x, 0, 0);
	Position after = vecAdd(tPos, delta);
	Position bottomRight = vecAdd(tTopLeft, tTextBoxSize);

	return (after.x > bottomRight.x);
}

PrismRectangle makeRectangleFromTexture(const TextureData& tTexture) {
	return makeRectangle(0, 0, tTexture.mTextureSize.x - 1, tTexture.mTextureSize.y - 1);
}

PrismRectangle scaleRectangle(const PrismRectangle& tRect, const Vector3D& tScale) {
	PrismRectangle ret;
	ret.topLeft.x = (int)(tRect.topLeft.x*tScale.x);
	ret.topLeft.y = (int)(tRect.topLeft.y*tScale.y);

	ret.bottomRight.x = (int)(tRect.bottomRight.x*tScale.x);
	ret.bottomRight.y = (int)(tRect.bottomRight.y*tScale.y);
	return ret;
}

PrismRectangle translateRectangle(const PrismRectangle& tRect, const Position& tOffset) {
	PrismRectangle ret = tRect;
	ret.topLeft.x += (int)tOffset.x;
	ret.topLeft.y += (int)tOffset.y;

	ret.bottomRight.x += (int)tOffset.x;
	ret.bottomRight.y += (int)tOffset.y;
	return ret;
}

Position getTextureMiddlePosition(const TextureData& tTexture) {
	return Vector3D(tTexture.mTextureSize.x / 2, tTexture.mTextureSize.y / 2, 0);
}

PrismRectangle makeRectangle(int x, int y, int w, int h) {
	PrismRectangle ret;
	ret.topLeft.x = x;
	ret.topLeft.y = y;
	ret.bottomRight.x = x + w;
	ret.bottomRight.y = y + h;
	return ret;
}

void printRectangle(const PrismRectangle& r) {
	logg("PrismRectangle");
	logDouble(r.topLeft.x);
	logDouble(r.topLeft.y);
	logDouble(r.bottomRight.x);
	logDouble(r.bottomRight.y);

}

void drawText(const char* tText, const Position& tPosition, TextSize tSize, Color tColor) {
	drawAdvancedText(tText, tPosition, makeFontSize(tSize, tSize), tColor, 0);
}

void drawAdvancedText(const char* tText, const Position& tPosition, const Vector3D& tFontSize, Color tColor, TextSize tBreakSize) {
	drawMultilineText(tText, tText, tPosition, tFontSize, tColor, makeFontSize(tBreakSize, 0), Vector3D(INF, INF, INF));
}

Vector3D makeFontSize(int x, int y) {
	Vector3D ret;
	ret.x = x;
	ret.y = y;
	ret.z = 1;
	return ret;
}