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
	BLEND_TYPE_ONE,
} BlendType;

typedef int TextSize;

void initDrawing();
void drawSprite(const TextureData& tTexture, const Position& tPos, const Rectangle& tTexturePosition);
void drawSpriteNoRectangle(const TextureData& tTexture, const Position& tTopLeft, const Position& tTopRight, const Position& tBottomLeft, const Position& tBottomRight, const Rectangle& tTexturePosition);
void drawText(const char* tText, const Position& tPosition, TextSize tSize, Color tColor);
void drawAdvancedText(const char* tText, const Position& tPosition, const Vector3D& tFontSize, Color tColor, TextSize tBreakSize);
void drawMultilineText(const char* tText, const char* tFullText, const Position& tPosition, const Vector3D& tFontSize, Color tColor, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize);
void drawTruetypeText(const char* tText, TruetypeFont tFont, const Position& tPosition, const Vector3DI& tTextSize, const Vector3D& tColor, double tTextBoxWidth, const GeoRectangle2D& tDrawRectangle);
void waitForScreen();
void startDrawing();
void stopDrawing();
bool isSkippingDrawing();
void setDrawingFrameSkippingEnabled(bool tIsEnabled);
void resetDrawingFrameStartTime();
void updateDrawingFrameStartTime(double tTimeDelta);

void disableDrawing();
void enableDrawing();

void scaleDrawing(double tFactor, const Position& tScalePosition);
void scaleDrawing2D(const Vector2D& tFactor, const Position2D& tScalePosition);
void scaleDrawing3D(const Vector3D& tFactor, const Position& tScalePosition);
void setDrawingBaseColorOffsetAdvanced(double r, double g, double b);
void setDrawingBaseColor(Color tColor);
void setDrawingBaseColorAdvanced(double r, double g, double b);
void setDrawingColorSolidity(int tIsSolid);
void setDrawingColorInversed(int tIsInversed);
void setDrawingColorFactor(double tColorFactor);
void setDrawingTransparency(double tAlpha);
void setDrawingDestinationTransparency(double tAlpha);
void setDrawingRotationZ(double tAngle, const Position2D& tPosition);
void setDrawingRotationZ(double tAngle, const Position& tPosition);
void setDrawingParametersToIdentity();
void setDrawingPunchThrough();
void setDrawingTransparent();
void setDrawingBlendType(BlendType tBlendType);

void pushDrawingTranslation(const Vector3D& tTranslation);
void pushDrawingRotationZ(double tAngle, const Vector3D& tCenter);

void popDrawingRotationZ();
void popDrawingTranslation();

Rectangle makeRectangle(int x, int y, int w, int h);
Rectangle makeRectangleFromTexture(const TextureData& tTexture);
Rectangle scaleRectangle(const Rectangle& tRect, const Vector3D& tScale);
Rectangle translateRectangle(const Rectangle& tRect, const Position& tOffset);
Position getTextureMiddlePosition(const TextureData& tTexture);
void printRectangle(const Rectangle& r);

Vector3D makeFontSize(int x, int y);

void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);
int hasToLinebreak(const char* tText, int tCurrent, const Position& tTopLeft, const Position& tPos, const Vector3D& tFontSize, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize);

void setPaletteFromARGB256Buffer(int tPaletteID, const Buffer& tBuffer);
void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, const Buffer& tBuffer);

double getRealFramerate();
