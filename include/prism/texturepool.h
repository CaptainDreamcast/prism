#pragma once

#include "texture.h"

namespace prism {

void setupTexturePool();
void shutdownTexturePool();

TextureData loadTextureFromPool(const char* tPath);
void unloadTextureFromPool(TextureData& tTexture);

void imguiTexturePool();

}
