#pragma once

#include <string>

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
	IntMap mGroups; // contains MugenSpriteFileGroup
	Vector mAllSprites; // contains MugenSpriteFileSprite
	Vector mPalettes; // contains Buffer
} MugenSpriteFile;

MugenSpriteFile loadMugenSpriteFile(char * tPath, int tHasPaletteFile, char* tOptionalPaletteFile);
MugenSpriteFile loadMugenSpriteFilePortraits(char * tPath, int tHasPaletteFile, char* tOptionalPaletteFile);

void unloadMugenSpriteFile(MugenSpriteFile* tFile);
void unloadMugenSpriteFileSprite(MugenSpriteFileSprite* tSprite);
MugenSpriteFile loadMugenSpriteFileWithoutPalette(std::string tPath);
MugenSpriteFile loadMugenSpriteFileWithoutPalette(char* tPath);
MugenSpriteFileSprite* getMugenSpriteFileTextureReference(MugenSpriteFile* tFile, int tGroup, int tSprite);

MugenSpriteFileSprite* loadSingleTextureFromPCXBuffer(Buffer tBuffer);

void setMugenSpriteFileReaderToBuffer();
void setMugenSpriteFileReaderToFileOperations();
void setMugenSpriteFileReaderToUsePalette(int tPaletteID);
void setMugenSpriteFileReaderToNotUsePalette();
void setMugenSpriteFileReaderCustomFunctionsAndForceARGB16(TextureData(*mCustomLoadTextureFromARGB16Buffer)(Buffer, int, int), TextureData(*mCustomLoadTextureFromARGB32Buffer)(Buffer, int, int), TextureData(*mCustomLoadPalettedTextureFrom8BitBuffer)(Buffer, int, int, int));
void setMugenSpriteFileReaderSubTextureSplit(int tSubTextureSplitMin, int tSubTextureSplitMax);

int hasMugenSprite(MugenSpriteFile* tSprites, int tGroup, int tSprite);