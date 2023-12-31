#pragma once

#include <string>
#include <map>

#include "datastructures.h"
#include "texture.h"


typedef struct {
	Vector2DI mOffset;
	TextureData mTexture;
} MugenSpriteFileSubSprite;

typedef struct {
	std::vector<MugenSpriteFileSubSprite> mTextures;
	TextureSize mOriginalTextureSize;
	int mIsLinked;
	int mIsLinkedTo;

	Vector2D mAxisOffset;
} MugenSpriteFileSprite;

typedef struct {
	std::map<int, MugenSpriteFileSprite> mSprites;
} MugenSpriteFileGroup;

typedef struct {
	int mGroup;
	int mItem;
	Buffer mBuffer;
} MugenSpriteFilePalette;

typedef struct {
	std::map<int, MugenSpriteFileGroup> mGroups;
	std::vector<MugenSpriteFileSprite*> mAllSprites;
	std::vector<MugenSpriteFilePalette> mPalettes;
	
	// group and item palette 1,1 is mapped to
	int mPaletteMappedGroup;
	int mPaletteMappedItem;
} MugenSpriteFile;

MugenSpriteFile loadMugenSpriteFile(const char * tPath, int tHasPaletteFile, const char* tOptionalPaletteFile);
MugenSpriteFile loadMugenSpriteFilePortraits(const char * tPath, int tHasPaletteFile, const char* tOptionalPaletteFile);

void unloadMugenSpriteFile(MugenSpriteFile* tFile);
void unloadMugenSpriteFileSprite(MugenSpriteFileSprite* tSprite);
MugenSpriteFile loadMugenSpriteFileWithoutPalette(const std::string& tPath);
MugenSpriteFile loadMugenSpriteFileWithoutPalette(const char* tPath);
MugenSpriteFileSprite* getMugenSpriteFileTextureReference(MugenSpriteFile* tFile, int tGroup, int tSprite);

MugenSpriteFileSprite loadSingleTextureFromPCXBuffer(const Buffer& tBuffer);

void setMugenSpriteFileReaderToBuffer();
void setMugenSpriteFileReaderToFileOperations();
void setMugenSpriteFileReaderToUsePalette(int tPaletteID);
void setMugenSpriteFileReaderToNotUsePalette();
void setMugenSpriteFileReaderCustomFunctionsAndForceARGB16(TextureData(*mCustomLoadTextureFromARGB16Buffer)(const Buffer&, int, int), TextureData(*mCustomLoadTextureFromARGB32Buffer)(const Buffer&, int, int), TextureData(*mCustomLoadPalettedTextureFrom8BitBuffer)(const Buffer&, int, int, int));
void setMugenSpriteFileReaderSubTextureSplit(int tSubTextureSplitMin, int tSubTextureSplitMax);

int hasMugenSprite(MugenSpriteFile* tSprites, int tGroup, int tSprite);
void remapMugenSpriteFilePalette(MugenSpriteFile* tSprites, const Vector2DI& tSource, const Vector2DI& tDestination, int tPaletteID);

void imguiMugenSpriteFile(MugenSpriteFile& tSprites, const std::string_view& tName);