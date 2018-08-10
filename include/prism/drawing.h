#pragma once

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
  COLOR_MAGENTA,
  COLOR_GRAY,
  COLOR_LIGHT_GRAY,
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

void initDrawing();
void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition);
void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor);
void drawAdvancedText(char* tText, Position tPosition, Vector3D tFontSize, Color tColor, TextSize tBreakSize);
void drawMultilineText(char* tText, char* tFullText, Position tPosition, Vector3D tFontSize, Color tColor, Vector3D tBreakSize, Vector3D tTextBoxSize);
void drawTruetypeText(char* tText, TruetypeFont tFont, Position tPosition, Vector3DI tTextSize, Vector3D tColor, double tTextBoxWidth);
void waitForScreen();
void startDrawing();
void stopDrawing();

void disableDrawing();
void enableDrawing();

void scaleDrawing(double tFactor, Position tScalePosition);
void scaleDrawing3D(Vector3D tFactor, Position tScalePosition);
void setDrawingBaseColor(Color tColor);
void setDrawingBaseColorAdvanced(double r, double g, double b);
void setDrawingTransparency(double tAlpha);
void setDrawingRotationZ(double tAngle, Position tPosition);
void setDrawingParametersToIdentity();
void setDrawingPunchThrough();
void setDrawingTransparent();
void setDrawingBlendType(BlendType tBlendType);

void pushDrawingTranslation(Vector3D tTranslation);
void pushDrawingRotationZ(double tAngle, Vector3D tCenter);

void popDrawingRotationZ();
void popDrawingTranslation();

Rectangle makeRectangle(int x, int y, int w, int h);
Rectangle makeRectangleFromTexture(TextureData tTexture);
Rectangle scaleRectangle(Rectangle tRect, Vector3D tScale);
Rectangle translateRectangle(Rectangle tRect, Position tOffset);
Position getTextureMiddlePosition(TextureData tTexture);
void printRectangle(Rectangle r);

Vector3D makeFontSize(int x, int y);

void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget);

void setPaletteFromARGB256Buffer(int tPaletteID, Buffer tBuffer);
void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, Buffer tBuffer);

double getRealFramerate();
