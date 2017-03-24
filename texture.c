#include "include/texture.h"

#include "include/file.h"
#include "include/log.h"

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
	FileHandler file;

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
