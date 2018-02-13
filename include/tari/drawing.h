#ifndef TARI_DRAWING
#define TARI_DRAWING

#include "physics.h"
#include "texture.h"
#include "file.h"

typedef struct {
  int x;
  int y;
} TexturePosition;

typedef struct {
  TexturePosition topLeft;
  TexturePosition bottomRight;
} Rectangle;

typedef enum {
  COLOR_BLACK,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW,
  COLOR_WHITE,
  COLOR_DARK_RED,
  COLOR_DARK_GREEN,
  COLOR_DARK_BLUE,
  COLOR_DARK_YELLOW,
  COLOR_CYAN,
} Color;

typedef enum {
	SPRITE_TYPE_TRANSPARENT,
	SPRITE_TYPE_PUNCH_THROUGH,
} SpriteType;

typedef enum {
	BLEND_TYPE_NORMAL,
	BLEND_TYPE_ADDITION,
	BLEND_TYPE_SUBTRACTION,
} BlendType;


typedef int TextSize;

fup void initDrawing();
fup void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition);
fup void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor);
fup void drawAdvancedText(char* tText, Position tPosition, Vector3D tFontSize, Color tColor, TextSize tBreakSize);
fup void drawMultilineText(char* tText, char* tFullText, Position tPosition, Vector3D tFontSize, Color tColor, Vector3D tBreakSize, Vector3D tTextBoxSize);
void drawTruetypeText(char* tText, TruetypeFont tFont, Position tPosition, Vector3DI tTextSize, Vector3D tColor, double tTextBoxWidth);
fup void waitForScreen();
fup void startDrawing();
fup void stopDrawing();

fup void disableDrawing();
fup void enableDrawing();

fup void scaleDrawing(double tFactor, Position tScalePosition);
fup void scaleDrawing3D(Vector3D tFactor, Position tScalePosition);
fup void setDrawingBaseColor(Color tColor);
fup void setDrawingBaseColorAdvanced(double r, double g, double b);
fup void setDrawingTransparency(double tAlpha);
fup void setDrawingRotationZ(double tAngle, Position tPosition);
fup void setDrawingParametersToIdentity();
fup void setDrawingPunchThrough();
fup void setDrawingTransparent();
fup void setDrawingBlendType(BlendType tBlendType);

fup void pushDrawingTranslation(Vector3D tTranslation);
fup void pushDrawingRotationZ(double tAngle, Vector3D tCenter);

fup void popDrawingRotationZ();
fup void popDrawingTranslation();

fup Rectangle makeRectangle(int x, int y, int w, int h);
fup Rectangle makeRectangleFromTexture(TextureData tTexture);
fup Rectangle scaleRectangle(Rectangle tRect, Vector3D tScale);
fup Rectangle translateRectangle(Rectangle tRect, Position tOffset);
fup Position getTextureMiddlePosition(TextureData tTexture);
fup void printRectangle(Rectangle r);

fup Vector3D makeFontSize(int x, int y);

fup void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

fup void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget);

void setPaletteFromARGB256Buffer(int tPaletteID, Buffer tBuffer);
void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, Buffer tBuffer);

#endif
