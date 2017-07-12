#ifndef TARI_TEXTURE
#define TARI_TEXTURE

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
} TextureData;

typedef struct {  // TODO: refactor completely from Dolmexica
  float mFilePositionX1;
  float mFilePositionY1;
  float mFilePositionX2;
  float mFilePositionY2;
} FontCharacterData;

fup TextureData loadTexturePKG(char tFileDir[]);
fup TextureData loadTexture(char tFileDir[]);
fup TextureData loadTextureFromARGB32Buffer(Buffer b, int tWidth, int tHeight);
fup void unloadTexture(TextureData tTexture);

fup void loadConsecutiveTextures(TextureData* tDst, char* tBaseFileDir, int tAmount);
fup TextureData getFontTexture();
fup FontCharacterData getFontCharacterData(char tChar);
fup void setFont(char tFileDirHeader[], char tFileDirTexture[]);

fup int getTextureHash(TextureData tTexture); 
fup int canLoadTexture(char* tPath);

#endif
