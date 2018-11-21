#pragma once

#include "memoryhandler.h"
#include "geometry.h"
#include "file.h"

typedef struct {
  int x;
  int y;
} TextureSize;

typedef struct {
  TextureSize mTextureSize;
  TextureMemory mTexture;
  int mHasPalette;
  int mPaletteID;
} TextureData;

typedef struct {  // TODO: refactor completely from Dolmexica
  float mFilePositionX1;
  float mFilePositionY1;
  float mFilePositionX2;
  float mFilePositionY2;
} FontCharacterData;

typedef void* TruetypeFont;

TextureData loadTexturePKG(char tFileDir[]);
TextureData loadTexture(char tFileDir[]);
TextureData loadTextureFromARGB16Buffer(Buffer b, int tWidth, int tHeight);
TextureData loadTextureFromTwiddledARGB16Buffer(Buffer b, int tWidth, int tHeight);
TextureData loadTextureFromARGB32Buffer(Buffer b, int tWidth, int tHeight);
TextureData loadTextureFromRawPNGBuffer(Buffer b, int tWidth, int tHeight);
TextureData loadPalettedTextureFrom8BitBuffer(Buffer b, int tPaletteID, int tWidth, int tHeight);
void unloadTexture(TextureData tTexture);

void loadConsecutiveTextures(TextureData* tDst, char* tBaseFileDir, int tAmount);
TextureData getFontTexture();
FontCharacterData getFontCharacterData(char tChar);
void setFont(char tFileDirHeader[], char tFileDirTexture[]);
TruetypeFont loadTruetypeFont(char* tName, double tSize);
void unloadTruetypeFont(TruetypeFont tFont);

int getTextureHash(TextureData tTexture); 
int canLoadTexture(char* tPath);

TextureSize makeTextureSize(int x, int y);
TextureData createWhiteTexture();

Buffer turnARGB32BufferIntoARGB16Buffer(Buffer tSrc);
Buffer twiddleTextureBuffer8(Buffer tBuffer, int tWidth, int tHeight);
Buffer twiddleTextureBuffer16(Buffer tBuffer, int tWidth, int tHeight);
