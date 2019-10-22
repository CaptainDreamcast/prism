#include "prism/texture.h"

#include<algorithm>

#ifdef DREAMCAST
#include <png/png.h>
#else
#include <png.h>
#endif

#include "prism/file.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/math.h"

using namespace std;

#define FONT_CHARACTER_AMOUNT 91

static int isFontDataLoaded;
static TextureData gFont;
static FontCharacterData gFontCharacterData[FONT_CHARACTER_AMOUNT];

void unloadFont() {
	if (!isFontDataLoaded)
		return;

	unloadTexture(gFont);
	memset(gFontCharacterData, 0, sizeof gFontCharacterData);

	isFontDataLoaded = 0;
}

static void loadFontHeader(const char* tFileDir) {
	FileHandler file;

	file = fileOpen(tFileDir, O_RDONLY);

	if (file == FILEHND_INVALID) {
		logError("Cannot open font header.");
		logErrorString(tFileDir);
		recoverFromError();
	}

	fileSeek(file, 0, 0);
	int i;
	for (i = 0; i < FONT_CHARACTER_AMOUNT; i++) {
		fileRead(file, &gFontCharacterData[i], sizeof gFontCharacterData[i]);
	}

	fileClose(file);
}

static void loadFontTexture(const char* tFileDir) {
	gFont = loadTexturePKG(tFileDir);
}

void setFont(const char* tFileDirHeader, const char* tFileDirTexture) {
	if (isFontDataLoaded) {
		unloadFont();
	}

	if (!isFile(tFileDirHeader)) {
		return;
	}

	loadFontHeader(tFileDirHeader);
	loadFontTexture(tFileDirTexture);

	isFontDataLoaded = 1;
}

void loadConsecutiveTextures(TextureData * tDst, const char * tBaseFileDir, int tAmount)
{
	int i;
	for (i = 0; i < tAmount; i++) {
		char path[1024];
		getPathWithNumberAffixedFromAssetPath(path, tBaseFileDir, i);
		tDst[i] = loadTexture(path);
	}
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

TextureSize makeTextureSize(int x, int y) {
	TextureSize ret;
	ret.x = x;
	ret.y = y;
	return ret;
}

TextureData createWhiteTexture() {
	int length = 16 * 16 * 4;
	uint8_t* data = (uint8_t*)allocMemory(length);
	memset(data, 0xFF, length);

	TextureData ret = loadTextureFromARGB32Buffer(makeBuffer(data, length), 16, 16);

	freeMemory(data);
	return ret;
}

TextureData createWhiteCircleTexture()
{
	int length = 16 * 16 * 4;
	uint8_t* data = (uint8_t*)allocMemory(length);
	memset(data, 0xFF, length);
	const std::vector<int> LINE_EMPTY_AMOUNT = {5, 3, 2, 1, 1};
	for (size_t y = 0; y < LINE_EMPTY_AMOUNT.size(); y++) {
		const auto width = LINE_EMPTY_AMOUNT[y];
		for (int x = 0; x < width; x++) {
			data[(y * 16 + x) * 4 + 3] = 0;
			data[(y * 16 + (15 - x)) * 4 + 3] = 0;
			data[((15 - y) * 16 + x) * 4 + 3] = 0;
			data[((15 - y) * 16 + (15 - x)) * 4 + 3] = 0;
		}
	}

	TextureData ret = loadTextureFromARGB32Buffer(makeBuffer(data, length), 16, 16);

	freeMemory(data);
	return ret;
}

Buffer turnARGB32BufferIntoARGB16Buffer(Buffer tSrc) {
	int dstSize = tSrc.mLength / 2;
	char* dst = (char*)allocMemory(dstSize);
	char* src = (char*)tSrc.mData;

	int n = dstSize / 2;
	int i;
	for(i = 0; i < n; i++) {
		int srcPos = 4*i;
		int dstPos = 2*i;

		uint8_t a = ((uint8_t)src[srcPos + 3]) >> 4;
		uint8_t r = ((uint8_t)src[srcPos + 2]) >> 4;
		uint8_t g = ((uint8_t)src[srcPos + 1]) >> 4;
		uint8_t b = ((uint8_t)src[srcPos + 0]) >> 4;
		
		dst[dstPos + 0] = (g << 4) | b;
		dst[dstPos + 1] = (a << 4) | r;
	}


	return makeBufferOwned(dst, dstSize);
}

/* Linear/iterative twiddling algorithm from Marcus' tatest */
#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )
#define MIN(a, b) ( (a)<(b)? (a):(b) )

/* This twiddling code is copied from pvr_texture.c, and the original
algorithm was written by Vincent Penne. */

Buffer twiddleTextureBuffer8(Buffer tBuffer, int tWidth, int tHeight) {
    int w = tWidth;
    int h = tHeight;
    int mini = min(w, h);
    int mask = mini - 1;
    uint8_t * pixels = (uint8_t *)tBuffer.mData;
	uint8_t * vtex = (uint8_t*)allocMemory(tBuffer.mLength);
    int x, y, yout;

    for(y = 0; y < h; y++) {
        yout = y;

        for(x = 0; x < w; x++) {
            vtex[TWIDOUT(x & mask, yout & mask) +
                 (x / mini + yout / mini)*mini * mini] = pixels[y * w + x];
        }
    }

    return makeBufferOwned(vtex, tBuffer.mLength);
}

Buffer twiddleTextureBuffer16(Buffer tBuffer, int tWidth, int tHeight) {
	int w = tWidth;
	int h = tHeight;
	int mini = min(w, h);
	int mask = mini - 1;
	uint16_t * pixels = (uint16_t *)tBuffer.mData;
	uint16_t * vtex = (uint16_t*)allocMemory(tBuffer.mLength);
	int x, y, yout;

	for (y = 0; y < h; y++) {
		yout = y;

		for (x = 0; x < w; x++) {
			vtex[TWIDOUT(x & mask, yout & mask) +
				(x / mini + yout / mini)*mini * mini] = pixels[y * w + x];
		}
	}

	return makeBufferOwned(vtex, tBuffer.mLength);
}

#ifdef DREAMCAST
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4611)
#endif
void saveRGB32ToPNG(Buffer b, int tWidth, int tHeight, const char* tFileDir) {
	char fullPath[1024];
	getFullPath(fullPath, tFileDir);
	FILE *fp = fopen(fullPath, "wb");
	if (!fp) {
		logErrorFormat("Unable to open file %s", tFileDir);
		return;
	}

	auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		logError("Unable to create png struct.");
		return;
	}

	auto info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		logError("Unable to create png info struct.");
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		logError("Exception writing png.");
		return;
	}

	png_init_io(png_ptr, fp);

	if (setjmp(png_jmpbuf(png_ptr))) {
		logError("Exception writing png.");
		return;
	}

	png_set_IHDR(png_ptr, info_ptr, tWidth, tHeight,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) {
		logError("Exception writing png.");
		return;
	}

	for (int y = tHeight - 1; y >= 0; y--) {
		png_bytep bytes = ((png_bytep)b.mData) + tWidth * 3 * y;
		png_write_rows(png_ptr, &bytes, 1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		logError("Exception writing png.");
		return;
	}

	png_write_end(png_ptr, NULL);

	fclose(fp);
}
#ifdef _WIN32
#pragma warning(pop)
#endif
