#include "prism/texture.h"

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#elif defined _WIN32
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include <string.h>

#include "prism/file.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/compression.h"



TextureData textureFromSurface(SDL_Surface* tSurface) {
	TextureData returnData;
	returnData.mTexture = allocTextureMemory(sizeof(SDLTextureData));
	returnData.mTextureSize.x = tSurface->w;
	returnData.mTextureSize.y = tSurface->h;
	returnData.mHasPalette = 0;
	Texture texture = returnData.mTexture->mData;
	
	texture->mSurface = SDL_ConvertSurfaceFormat(tSurface, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(tSurface);

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &texture->mTexture);
	glBindTexture(GL_TEXTURE_2D, texture->mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->mSurface->w, texture->mSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->mSurface->pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return returnData;
}

static TextureData loadTexturePNG(char* tFileDir) {
	
	Buffer b = fileToBuffer(tFileDir);
	SDL_RWops* memStream = SDL_RWFromMem(b.mData, b.mLength);
	SDL_Surface* loadedSurface = IMG_Load_RW(memStream, 1);
	freeBuffer(b);
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
		TextureData ret;
		ret.mHasPalette = 0;
		return ret;
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

TextureData loadTextureFromARGB16Buffer(Buffer b, int tWidth, int tHeight)
{
	uint32_t amask = 0x0000f000;
	uint32_t rmask = 0x00000f00;
	uint32_t gmask = 0x000000f0;
	uint32_t bmask = 0x0000000f;

	int depth = 16;
	int pitch = 2 * tWidth;

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(b.mData, tWidth, tHeight, depth, pitch, rmask, gmask, bmask, amask);
	return textureFromSurface(surface);
}

TextureData loadTextureFromARGB32Buffer(Buffer b, int tWidth, int tHeight) {
	uint32_t amask = 0xff000000;
	uint32_t rmask = 0x00ff0000;
	uint32_t gmask = 0x0000ff00;
	uint32_t bmask = 0x000000ff;
	
	int depth = 32;
	int pitch = 4 * tWidth;

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(b.mData, tWidth, tHeight, depth, pitch, rmask, gmask, bmask, amask);
	return textureFromSurface(surface);
}

TextureData loadTextureFromRawPNGBuffer(Buffer b, int tWidth, int tHeight) {
	(void)tWidth;
	(void)tHeight;
	SDL_RWops* memStream = SDL_RWFromMem(b.mData, b.mLength);
	if (!memStream) {
		logError("Unable to create memory stream.");
		logErrorString(SDL_GetError());
		abortSystem();
	}

	SDL_Surface* surface = IMG_LoadPNG_RW(memStream);
	if (!surface) {
		logError("Unable to create surface.");
		logErrorString(SDL_GetError());
		abortSystem();
	}

	return textureFromSurface(surface);
}

extern SDL_Color* getSDLColorPalette(int tIndex);

TextureData loadPalettedTextureFrom8BitBuffer(Buffer b, int tPaletteID, int tWidth, int tHeight) {
	uint32_t size = tWidth * tHeight * 4;
	uint8_t* data = allocMemory(tWidth * tHeight * 4);
	uint8_t* src = b.mData;

	SDL_Color* colors = getSDLColorPalette(tPaletteID);
	int32_t amount = size / 4;
	int i;
	for (i = 0; i < amount; i++) {
		int pid = src[i];
		data[i * 4 + 0] = colors[pid].r;
		data[i * 4 + 1] = colors[pid].g;
		data[i * 4 + 2] = colors[pid].b;
		data[i * 4 + 3] = colors[pid].a;
	}

	Buffer newBuffer = makeBufferOwned(data, size);
	TextureData ret = loadTextureFromARGB32Buffer(newBuffer, tWidth, tHeight);
	freeBuffer(newBuffer);

	return ret;

	// TODO: fix
	/*

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(b.mData, tWidth, tHeight, 8, tWidth, 0, 0, 0, 0);

	TextureData returnData;
	returnData.mTexture = allocTextureMemory(sizeof(SDLTextureData));
	returnData.mTextureSize.x = surface->w;
	returnData.mTextureSize.y = surface->h;
	returnData.mHasPalette = 1;
	returnData.mPaletteID = tPaletteID;
	Texture texture = returnData.mTexture->mData;

	texture->mSurface = surface;

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &texture->mTexture);
	glBindTexture(GL_TEXTURE_2D, texture->mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture->mSurface->w, texture->mSurface->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture->mSurface->pixels);

	glBindTexture(GL_TEXTURE_2D, last_texture);

	return returnData;
	*/
}

TruetypeFont loadTruetypeFont(char * tName, double tSize)
{

	char path[1024];
	if (isFile(tName)) {
		getFullPath(path, tName);
	}
	else {
		sprintf(path, "C:/Windows/Fonts/%s", tName); // TODO: properly
		if (!isFile(path)) {
			logError("Unable to open font file.");
			logErrorString(tName);
			logErrorString(path);
			abortSystem();
		}
	}

	printf("%s\n", path);
	
	TTF_Font* font = TTF_OpenFont(path, (int)tSize);
	printf("err: %s\n", TTF_GetError());

	return font;
}

void unloadTruetypeFont(TruetypeFont tFont)
{
	TTF_Font* font = tFont;
	TTF_CloseFont(font);
}
