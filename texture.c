#include "include/texture.h"

#include <kos.h>
#include <kos/string.h>

// TODO: remove quicklz and remove it with something suitable
#include "include/quicklz.h"

#include "include/file.h"

#include "include/log.h"
#include "include/memoryhandler.h"

#define HEADER_SIZE_KMG 64
// TODO: use kmg.h from KOS

TextureData loadTexturePKG(char tFileDir[]) {

  TextureData returnData;

  qlz_state_decompress *state_decompress = (qlz_state_decompress *) allocMemory(sizeof(qlz_state_decompress));
  size_t bufferLength;
  file_t pkgFile;
  char* mipMapData;
  char* kmgData;

  pkgFile = fileOpen(tFileDir, O_RDONLY);

  if (pkgFile == FILEHND_INVALID) {
    logError("Couldn't open file: Try returning to menu...");
    logErrorString(tFileDir);
    arch_menu();
  }

  bufferLength = fileTotal(pkgFile);
  debugInteger(bufferLength);

  mipMapData = fileMemoryMap(pkgFile);

  bufferLength = qlz_size_decompressed(mipMapData);
  debugInteger(bufferLength);

  kmgData = (char*) allocMemory(bufferLength);

  // decompress and write result
  bufferLength = qlz_decompress(mipMapData, kmgData, state_decompress);
  debugInteger(bufferLength);

  fileClose(pkgFile);
  freeMemory(state_decompress);

  returnData.mTextureSize.x = 0;
  returnData.mTextureSize.y = 0;

  memcpy4(&returnData.mTextureSize.x, kmgData + 16, sizeof returnData.mTextureSize.x);
  memcpy4(&returnData.mTextureSize.y, kmgData + 20, sizeof returnData.mTextureSize.y);

  returnData.mTexture = allocTextureMemory(bufferLength - HEADER_SIZE_KMG);

  sq_cpy(returnData.mTexture, kmgData + HEADER_SIZE_KMG, bufferLength - HEADER_SIZE_KMG);

  freeMemory(kmgData);

  return returnData;
}

void unloadTexture(TextureData tTexture) {
  freeTextureMemory(tTexture.mTexture);
}

#define FONT_CHARACTER_AMOUNT 91

int isFontDataLoaded;
TextureData gFont;
FontCharacterData gFontCharacterData[FONT_CHARACTER_AMOUNT];

void unloadFont() {
  if (!isFontDataLoaded)
    return;

  unloadTexture(gFont);
  memset(gFontCharacterData, 0, sizeof gFontCharacterData);

  isFontDataLoaded = 0;
}

void loadFontHeader(char tFileDir[]) {
  file_t file;

  file = fileOpen(tFileDir, O_RDONLY);
  fileSeek(file, 0, 0);
  int i;
  for (i = 0; i < FONT_CHARACTER_AMOUNT; i++) {
    fileRead(file, &gFontCharacterData[i], sizeof gFontCharacterData[i]);
  }

  fileClose(file);
}

void loadFontTexture(char tFileDir[]) {
  gFont = loadTexturePKG(tFileDir);
}

void setFont(char tFileDirHeader[], char tFileDirTexture[]) {
  if (isFontDataLoaded) {
    unloadFont();
  }

  loadFontHeader(tFileDirHeader);
  loadFontTexture(tFileDirTexture);

  isFontDataLoaded = 1;
}

TextureData getFontTexture() {
  return gFont;
}

FontCharacterData getFontCharacterData(char tChar) {
  int i;
  if (tChar < ' ' || tChar > 'z')
    i = 0;
  else
    i = tChar - ' ';

  return gFontCharacterData[i];
}

int getAvailableTextureMemory() {
	return pvr_mem_available();
}
