#pragma once

#include "physics.h"
#include "texture.h"
#include "file.h"

namespace prism {

typedef struct {
  int x;
  int y;
} TexturePosition;

typedef struct {
  TexturePosition topLeft;
  TexturePosition bottomRight;
} PrismRectangle;

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
void drawSprite(const TextureData& tTexture, const Position& tPos, const PrismRectangle& tTexturePosition);
void drawSpriteNoRectangle(const TextureData& tTexture, const Position& tTopLeft, const Position& tTopRight, const Position& tBottomLeft, const Position& tBottomRight, const PrismRectangle& tTexturePosition);
void drawText(const char* tText, const Position& tPosition, TextSize tSize, Color tColor);
void drawAdvancedText(const char* tText, const Position& tPosition, const Vector3D& tFontSize, Color tColor, TextSize tBreakSize);
void drawMultilineText(const char* tText, const char* tFullText, const Position& tPosition, const Vector3D& tFontSize, Color tColor, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize);
void drawTruetypeText(const char* tText, TruetypeFont tFont, const Position& tPosition, const Vector3DI& tTextSize, const Vector3D& tColor, float tTextBoxWidth, const GeoRectangle2D& tDrawRectangle);
void waitForScreen();
void startDrawing();
void stopDrawing();
void waitForRendering();
bool isSkippingDrawing();
void setDrawingFrameSkippingEnabled(bool tIsEnabled);
void resetDrawingFrameStartTime();
void updateDrawingFrameStartTime(float tTimeDelta);

void disableDrawing();
void enableDrawing();

void scaleDrawing(float tFactor, const Position& tScalePosition);
void scaleDrawing2D(const Vector2D& tFactor, const Position2D& tScalePosition);
void scaleDrawing3D(const Vector3D& tFactor, const Position& tScalePosition);
void setDrawingBaseColorOffsetAdvanced(float r, float g, float b);
void setDrawingBaseColor(Color tColor);
void setDrawingBaseColorAdvanced(float r, float g, float b);
void setDrawingColorSolidity(int tIsSolid);
void setDrawingColorInversed(int tIsInversed);
void setDrawingColorFactor(float tColorFactor);
void setDrawingTransparency(float tAlpha);
void setDrawingDestinationTransparency(float tAlpha);
void setDrawingRotationZ(float tAngle, const Position2D& tPosition);
void setDrawingRotationZ(float tAngle, const Position& tPosition);
void setDrawingParametersToIdentity();
void setDrawingPunchThrough();
void setDrawingTransparent();
void setDrawingBlendType(BlendType tBlendType);

void pushDrawingTranslation(const Vector3D& tTranslation);
void pushDrawingRotationZ(float tAngle, const Vector3D& tCenter);

void popDrawingRotationZ();
void popDrawingTranslation();

PrismRectangle makeRectangle(int x, int y, int w, int h);
PrismRectangle makeRectangleFromTexture(const TextureData& tTexture);
PrismRectangle scaleRectangle(const PrismRectangle& tRect, const Vector3D& tScale);
PrismRectangle translateRectangle(const PrismRectangle& tRect, const Position& tOffset);
Position getTextureMiddlePosition(const TextureData& tTexture);
void printRectangle(const PrismRectangle& r);

Vector3D makeFontSize(int x, int y);

void getRGBFromColor(Color tColor, float* tR, float* tG, float* tB);
int hasToLinebreak(const char* tText, int tCurrent, const Position& tTopLeft, const Position& tPos, const Vector3D& tFontSize, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize);

void setPaletteFromARGB256Buffer(int tPaletteID, const Buffer& tBuffer);
void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, const Buffer& tBuffer);

float getRealFramerate();

#ifdef _WIN32
void imguiDrawingHardware();
#endif

}