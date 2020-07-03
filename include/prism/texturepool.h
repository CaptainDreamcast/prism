#pragma once

#include "texture.h"

void setupTexturePool();
void shutdownTexturePool();

TextureData loadTextureFromPool(const char* tPath);
void unloadTextureFromPool(TextureData& tTexture);
