#include "tari/texture.h"

#include <SDL.h>
#include <SDL_image.h>
#include <string.h>

#include "tari/file.h"
#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"
#include "tari/math.h"
#include "tari/compression.h"



extern SDL_Renderer* gRenderer;

static TextureData textureFromSurface(SDL_Surface* tSurface) {
	
	SDL_Texture* newTexture;
	newTexture = SDL_CreateTextureFromSurface(gRenderer, tSurface);
	if (newTexture == NULL)
	{
		logError("Unable to create texture");
		logErrorString(SDL_GetError());
		abortSystem();
	}

	SDL_FreeSurface(tSurface);

	TextureData returnData;
	returnData.mTexture = allocTextureMemory(sizeof(SDLTextureData));
	Texture texture = returnData.mTexture->mData;
	texture->mTexture = newTexture;
	int access;
	Uint32 format;
	SDL_QueryTexture(newTexture, &format, &access, &returnData.mTextureSize.x, &returnData.mTextureSize.y);

	return returnData;
}

static TextureData loadTexturePNG(char* tFileDir) {
	

	SDL_Surface* loadedSurface = IMG_Load(tFileDir);
	if (loadedSurface == NULL)
	{
		logError("Unable to load file:");
		logErrorString(tFileDir);
		logErrorString(IMG_GetError());
		abortSystem();
	}

	return textureFromSurface(loadedSurface);
}



#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )
#define MIN(a, b) ( (a)<(b)? (a):(b) )


static void untwiddle(uint16_t * src, uint16_t * output, int w, int h) {
	int min = MIN(w, h);
	int mask = min - 1;
	uint16_t * pixels = output;
	uint16_t * vtex = src;
	int x, y, yout;

	for (y = 0; y < h; y++) {
		yout = y;

		for (x = 0; x < w; x++) {
			pixels[y * w + x] = vtex[TWIDOUT(x & mask, yout & mask) +
				(x / min + yout / min)*min * min];
		}
	}
}

typedef struct kmg_header {
	uint32_t		magic;		/* Magic code */
	uint32_t		version;	/* Version code */
	uint32_t		platform;	/* Platform specifier (major format) */
	uint32_t		format;		/* Image (minor) format spec */
	uint32_t		width;		/* Image width */
	uint32_t		height;		/* Image height */
	uint32_t		byte_count;	/* Image's data size in bytes */
	uint8_t		padding[36];	/* Pad to a 64-byte header (all zeros) */
} KMGHeader;

#define KMG_MAGIC	0x00474d4b /* 'KMG\0' */


static SDL_Surface* makeSurfaceFromUntwiddledTexture(Buffer b, KMGHeader tHeader) {

	uint32_t rmask = 0x0f00;
	uint32_t gmask = 0x00f0;
	uint32_t bmask = 0x000f;
	uint32_t amask = 0xf000;
	int depth = 16;
	int pitch = 2 * tHeader.width;

	return SDL_CreateRGBSurfaceFrom(b.mData, tHeader.width, tHeader.height, depth, pitch, rmask, gmask, bmask, amask);
}

static KMGHeader untwiddleBufferAndReturnHeader(Buffer* tBuffer) {
	Buffer src = *tBuffer;
	Buffer dst = *tBuffer;

	BufferPointer p = getBufferPointer(src);
	KMGHeader hdr;
	readFromBufferPointer(&hdr, &p, sizeof hdr);

	dst.mLength = dst.mLength - sizeof hdr;
	dst.mData = allocMemory(dst.mLength);

	if (hdr.magic != KMG_MAGIC) {
		logError("Unable to open KMG file.");
		logErrorHex(hdr.magic);
		abortSystem();
	}

	untwiddle((uint16_t*)p, dst.mData, hdr.width, hdr.height);

	freeBuffer(src);
	*tBuffer = dst;

	return hdr;
}

static TextureData loadTexturePKGWindows(char* tFileDir) {
	Buffer b = fileToBuffer(tFileDir);
	decompressBuffer(&b);
	KMGHeader hdr = untwiddleBufferAndReturnHeader(&b);
	SDL_Surface* s = makeSurfaceFromUntwiddledTexture(b, hdr);
	return textureFromSurface(s);
}

TextureData loadTexturePKG(char* tFileDir) {
	char pngPath[1024];

	strcpy(pngPath, tFileDir);
	int len = strlen(pngPath);
	pngPath[len - 2] = 'n';


	char fullFileName[1024];
	char fullFileNamePNG[1024];
	getFullPath(fullFileName, tFileDir);
	getFullPath(fullFileNamePNG, pngPath);

	// TODO: Properly decode PKG files; currently assume we have PNG instead
	if (isFile(pngPath)) {
		return loadTexturePNG(fullFileNamePNG);
	}
	else {
		return loadTexturePKGWindows(fullFileName);
	}


}

TextureData loadTexture(char* tFileDir) {
	char* fileExt = getFileExtension(tFileDir);

	if (!strcmp("pkg", fileExt)) {
		return loadTexturePKG(tFileDir);
	}
	else {
		logError("Unable to identify texture file type.");
		logErrorString(fileExt);
		abortSystem();
	}
}

void unloadTexture(TextureData tTexture) {
	freeTextureMemory(tTexture.mTexture);
}

int getTextureHash(TextureData tTexture) {
	return (int)tTexture.mTexture;
}

int canLoadTexture(char* tPath) {
	char* fileExt = getFileExtension(tPath);

	if (!strcmp("pkg", fileExt)) {
		char path[1024];
		strcpy(path, tPath);
		char* newFileExt = getFileExtension(path);
		strcpy(newFileExt, "png");
		return isFile(path) || isFile(tPath);
	}

	return 0;
}