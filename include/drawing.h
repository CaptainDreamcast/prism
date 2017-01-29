#ifndef TARI_DRAWING
#define TARI_DRAWING

#include "physics.h"
#include "texture.h"

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
  COLOR_WHITE
} Color;

typedef int TextSize;

void initDrawing();
void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition);
void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor);
void waitForScreen();
void startDrawing();
void stopDrawing();

void scaleDrawing(double tFactor, Position tScalePosition);
void scaleDrawing3D(Vector3D tFactor, Position tScalePosition);
void setDrawingBaseColor(Color tColor);
void setDrawingTransparency(double tAlpha);
void setDrawingRotationZ(double tAngle, Position tPosition);
void setDrawingParametersToIdentity();

Rectangle makeRectangle(int x, int y, int w, int h);
Rectangle makeRectangleFromTexture(TextureData tTexture);
Position getTextureMiddlePosition(TextureData tTexture);
void printRectangle(Rectangle r);



#endif
