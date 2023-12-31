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

typedef struct { 
  float mFilePositionX1;
  float mFilePositionY1;
  float mFilePositionX2;
  float mFilePositionY2;
} FontCharacterData;

typedef void* TruetypeFont;

TextureData loadTexturePNG(const char* tFileDir);
TextureData loadTexturePKG(const char* tFileDir);
TextureData loadTexture(const char* tFileDir);
TextureData loadTextureFromARGB16Buffer(const Buffer& b, int tWidth, int tHeight);
TextureData loadTextureFromTwiddledARGB16Buffer(const Buffer& b, int tWidth, int tHeight);
TextureData loadTextureFromARGB32Buffer(const Buffer& b, int tWidth, int tHeight);
TextureData loadTextureFromRawPNGBuffer(const Buffer& b, int tWidth, int tHeight);
TextureData loadPalettedTextureFrom8BitBuffer(const Buffer& b, int tPaletteID, int tWidth, int tHeight);
void unloadTexture(TextureData& tTexture);

void loadConsecutiveTextures(TextureData* tDst, const char* tBaseFileDir, int tAmount);
TextureData getFontTexture();
FontCharacterData getFontCharacterData(char tChar);
void setFont(const char* tFileDirHeader, const char* tFileDirTexture);
TruetypeFont loadTruetypeFont(const char* tName, double tSize);
void unloadTruetypeFont(TruetypeFont tFont);

int getTextureHash(const TextureData& tTexture); 
int canLoadTexture(const char* tPath);

TextureSize makeTextureSize(int x, int y);
TextureData createWhiteTexture();
TextureData createWhiteCircleTexture();

Buffer turnARGB32BufferIntoARGB16Buffer(const Buffer& tSrc);
Buffer twiddleTextureBuffer8(const Buffer& tBuffer, int tWidth, int tHeight);
Buffer twiddleTextureBuffer16(const Buffer& tBuffer, int tWidth, int tHeight);

void saveScreenShot(const char* tFileDir);
void saveRGB32ToPNG(const Buffer& b, int tWidth, int tHeight, const char* tFileDir);

void imguiTextureData(const std::string_view& tName, const TextureData& tTextureData);