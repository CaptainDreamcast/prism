#pragma once

#include "texture.h"

void setupTexturePool();
void shutdownTexturePool();

TextureData loadTextureFromPool(char* tPath);
void unloadTextureFromPool(TextureData tTexture);
