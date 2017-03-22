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

TextureData loadTexturePKG(char tFileDir[]);
TextureData loadTexture(char tFileDir[]);
void unloadTexture(TextureData tTexture);

TextureData getFontTexture();
FontCharacterData getFontCharacterData(char tChar);
void setFont(char tFileDirHeader[], char tFileDirTexture[]);

int getAvailableTextureMemory();

#endif
