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

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition);
void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor);
void waitForScreen();
void startDrawing();
void stopDrawing();

#endif
