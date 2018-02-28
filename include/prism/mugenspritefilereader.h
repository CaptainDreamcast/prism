#pragma once

#include "datastructures.h"
#include "texture.h"


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


MugenSpriteFile loadMugenSpriteFile(char * tPath, int tPreferredPalette, int tHasPaletteFile, char* tOptionalPaletteFile);
MugenSpriteFile loadMugenSpriteFilePortraits(char * tPath, int tPreferredPalette, int tHasPaletteFile, char* tOptionalPaletteFile);

void unloadMugenSpriteFile(MugenSpriteFile* tFile);
void unloadMugenSpriteFileSprite(MugenSpriteFileSprite* tSprite);
MugenSpriteFile loadMugenSpriteFileWithoutPalette(char* tPath);
MugenSpriteFileSprite* getMugenSpriteFileTextureReference(MugenSpriteFile* tFile, int tGroup, int tSprite);

MugenSpriteFileSprite* loadSingleTextureFromPCXBuffer(Buffer tBuffer);

void setMugenSpriteFileReaderToBuffer();
void setMugenSpriteFileReaderToFileOperations();
void setMugenSpriteFileReaderToUsePalette(int tPaletteID);
void setMugenSpriteFileReaderToNotUsePalette();