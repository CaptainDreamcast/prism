#pragma once

#include "tari/datastructures.h"
#include "tari/texture.h"


typedef struct {
	Vector3DI mOffset;
	TextureData mTexture;
} MugenSpriteFileSubSprite;

typedef struct {
	List mTextures; // MugenSpriteFileSubSprite
	TextureSize mOriginalTextureSize;
	int mIsLinked;
	int mIsLinkedTo;

	Vector3D mAxisOffset;
} MugenSpriteFileSprite;

typedef struct {
	IntMap mSprites;

} MugenSpriteFileGroup;

typedef struct {
	IntMap mGroups;
	Vector mAllSprites;
	Vector mPalettes;
} MugenSpriteFile;


fup MugenSpriteFile loadMugenSpriteFile(char * tPath, int tPreferredPalette, int tHasPaletteFile, char* tOptionalPaletteFile);
fup MugenSpriteFile loadMugenSpriteFilePortraits(char * tPath, int tPreferredPalette, int tHasPaletteFile, char* tOptionalPaletteFile);

void unloadMugenSpriteFile(MugenSpriteFile* tFile);
void unloadMugenSpriteFileSprite(MugenSpriteFileSprite* tSprite);
fup MugenSpriteFile loadMugenSpriteFileWithoutPalette(char* tPath);
fup MugenSpriteFileSprite* getMugenSpriteFileTextureReference(MugenSpriteFile* tFile, int tGroup, int tSprite);

MugenSpriteFileSprite* loadSingleTextureFromPCXBuffer(Buffer tBuffer);