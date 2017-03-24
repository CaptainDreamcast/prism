#ifndef TARI_TEXTURE
#define TARI_TEXTURE

#define _WINDOWS_

#ifdef DREAMCAST
#include <kos.h>

typedef pvr_ptr_t Texture;

#elif defined _WINDOWS_
#include <SDL.h>

typedef SDL_Texture* Texture;

#endif

#include "common/header.h"

typedef struct {
  int x;
  int y;
} TextureSize;

typedef struct {
  TextureSize mTextureSize;
  Texture mTexture;
} TextureData;

typedef struct {  // TODO: refactor completely from Dolmexica
  float mFilePositionX1;
  float mFilePositionY1;
  float mFilePositionX2;
  float mFilePositionY2;
} FontCharacterData;

fup TextureData loadTexturePKG(char tFileDir[]);
fup TextureData loadTexture(char tFileDir[]);
fup void unloadTexture(TextureData tTexture);

fup TextureData getFontTexture();
fup FontCharacterData getFontCharacterData(char tChar);
fup void setFont(char tFileDirHeader[], char tFileDirTexture[]);

fup int getAvailableTextureMemory();

#endif
