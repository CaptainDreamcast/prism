#ifndef TARI_TEXTUREPOOL_H
#define TARI_TEXTUREPOOL_H

#include "texture.h"

fup void setupTexturePool();
fup void shutdownTexturePool();

fup TextureData loadTextureFromPool(char* tPath);
fup void unloadTextureFromPool(TextureData tTexture);

#endif