#include "prism/texture.h"

#include <stdlib.h>
#include <string.h>

#include "prism/file.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"

namespace prism {

	static struct {
		unsigned int mNextTextureID = 1;
	} gTextureLinuxData;

	static TextureData makeHeadlessTexture(int tWidth, int tHeight) {
		TextureData ret;
		ret.mTextureSize.x = tWidth;
		ret.mTextureSize.y = tHeight;
		ret.mTexture = allocTextureMemory(sizeof(GLTextureData));
		GLTextureData* data = (GLTextureData*)ret.mTexture->mData;
		data->mTexture = gTextureLinuxData.mNextTextureID++;
		ret.mHasPalette = 0;
		ret.mPaletteID = 0;
		return ret;
	}

	static int parsePNGSize(const Buffer& b, int* oWidth, int* oHeight) {
		if (b.mLength < 24) return 0;
		const unsigned char* d = (const unsigned char*)b.mData;
		static const unsigned char signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
		if (memcmp(d, signature, 8)) return 0;
		*oWidth = (d[16] << 24) | (d[17] << 16) | (d[18] << 8) | d[19];
		*oHeight = (d[20] << 24) | (d[21] << 16) | (d[22] << 8) | d[23];
		return 1;
	}

	TextureData loadTexturePNG(const char* tFileDir) {
		Buffer b = fileToBuffer(tFileDir);
		int width = 1, height = 1;
		if (!parsePNGSize(b, &width, &height)) {
			logWarningFormat("Unable to parse PNG size for %s, using 1x1.", tFileDir);
		}
		freeBuffer(b);
		return makeHeadlessTexture(width, height);
	}

	TextureData loadTexture(const char* tFileDir) {
		return loadTexturePNG(tFileDir);
	}

	TextureData loadTexturePKG(const char* tFileDir) {
		char pngPath[1024];
		strcpy(pngPath, tFileDir);
		char* fileExt = getFileExtension(pngPath);
		if (fileExt) strcpy(fileExt, "png");
		return loadTexturePNG(pngPath);
	}

	TextureData loadTextureFromARGB16Buffer(const Buffer& /*b*/, int tWidth, int tHeight) { return makeHeadlessTexture(tWidth, tHeight); }
	TextureData loadTextureFromTwiddledARGB16Buffer(const Buffer& /*b*/, int tWidth, int tHeight) { return makeHeadlessTexture(tWidth, tHeight); }
	TextureData loadTextureFromARGB32Buffer(const Buffer& /*b*/, int tWidth, int tHeight) { return makeHeadlessTexture(tWidth, tHeight); }
	TextureData loadTextureFromRawPNGBuffer(const Buffer& b, int tWidth, int tHeight) {
		int width = tWidth, height = tHeight;
		parsePNGSize(b, &width, &height);
		return makeHeadlessTexture(width, height);
	}
	TextureData loadPalettedTextureFrom8BitBuffer(const Buffer& /*b*/, int tPaletteID, int tWidth, int tHeight) {
		auto ret = makeHeadlessTexture(tWidth, tHeight);
		ret.mHasPalette = 1;
		ret.mPaletteID = tPaletteID;
		return ret;
	}

	void unloadTexture(TextureData& tTexture) {
		freeTextureMemory(tTexture.mTexture);
	}

	TruetypeFont loadTruetypeFont(const char* /*tName*/, float /*tSize*/) {
		return (TruetypeFont)malloc(1);
	}

	void unloadTruetypeFont(TruetypeFont tFont) {
		free(tFont);
	}

	int getTextureHash(const TextureData& tTexture) {
		GLTextureData* data = (GLTextureData*)tTexture.mTexture->mData;
		return (int)data->mTexture;
	}

	int canLoadTexture(const char* tPath) {
		return isFile(tPath);
	}

	void saveScreenShot(const char* /*tFileDir*/) {}

	void copyScreenShotToClipboard() {} // UNSUPPORTED

}
