#include "prism/texture.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#ifdef __EMSCRIPTEN__
#include <SDL_image.h>
#include <SDL_ttf.h>
#elif defined _WIN32
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include <string.h>
#include <sstream>

#include "prism/file.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/compression.h"

static TextureData textureFromSurface(SDL_Surface* tSurface) {
	TextureData returnData;
	returnData.mTexture = allocTextureMemory(sizeof(SDLTextureData));
	returnData.mTextureSize.x = tSurface->w;
	returnData.mTextureSize.y = tSurface->h;
	returnData.mHasPalette = 0;
	Texture texture = (Texture)returnData.mTexture->mData;
	
	auto surface = SDL_ConvertSurfaceFormat(tSurface, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(tSurface);

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &texture->mTexture);
	glBindTexture(GL_TEXTURE_2D, texture->mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture);

	SDL_FreeSurface(surface);

	return returnData;
}

TextureData loadTexturePNG(const char* tFileDir) {
	
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


static SDL_Surface* makeSurfaceFromUntwiddledTexture(const Buffer& b, const KMGHeader& tHeader) {

	uint32_t rmask = 0x0f00;
	uint32_t gmask = 0x00f0;
	uint32_t bmask = 0x000f;
	uint32_t amask = 0xf000;
	int depth = 16;
	int pitch = 2 * tHeader.width;

	return SDL_CreateRGBSurfaceFrom(b.mData, tHeader.width, tHeader.height, depth, pitch, rmask, gmask, bmask, amask);
}

static Buffer untwiddleBuffer(const Buffer& tBuffer, uint32_t tWidth, uint32_t tHeight) {
	uint16_t* dst = (uint16_t*)allocMemory(tBuffer.mLength);
	uint32_t dstLength = tBuffer.mLength;

	untwiddle((uint16_t*)tBuffer.mData, dst, tWidth, tHeight);

	return makeBufferOwned(dst, dstLength);
}

static KMGHeader untwiddleKMGBufferAndReturnHeader(Buffer* tBuffer) {
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

	untwiddle((uint16_t*)p, (uint16_t*)dst.mData, hdr.width, hdr.height);

	freeBuffer(src);
	*tBuffer = dst;

	return hdr;
}

static TextureData loadTexturePKGWindows(const char* tFileDir) {
	Buffer b = fileToBuffer(tFileDir);
	decompressBufferZSTD(&b);
	KMGHeader hdr = untwiddleKMGBufferAndReturnHeader(&b);
	SDL_Surface* s = makeSurfaceFromUntwiddledTexture(b, hdr);
	return textureFromSurface(s);
}

TextureData loadTexturePKG(const char* tFileDir) {
	char pngPath[1024];

	strcpy(pngPath, tFileDir);
	int len = strlen(pngPath);
	pngPath[len - 2] = 'n';


	char fullFileName[1024];
	char fullFileNamePNG[1024];
	getFullPath(fullFileName, tFileDir);
	getFullPath(fullFileNamePNG, pngPath);

	if (isFile(pngPath)) {
		return loadTexturePNG(fullFileNamePNG);
	}
	else {
		return loadTexturePKGWindows(fullFileName);
	}


}

TextureData loadTexture(const char* tFileDir) {
	const char* fileExt = getFileExtension(tFileDir);

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

void unloadTexture(TextureData& tTexture) {
	freeTextureMemory(tTexture.mTexture);
}

int getTextureHash(const TextureData& tTexture) {
	return (int)tTexture.mTexture;
}

int canLoadTexture(const char* tPath) {
	const char* fileExt = getFileExtension(tPath);

	if (!strcmp("pkg", fileExt)) {
		char path[1024];
		strcpy(path, tPath);
		char* newFileExt = getFileExtension(path);
		strcpy(newFileExt, "png");
		return isFile(path) || isFile(tPath);
	}

	return 0;
}

TextureData loadTextureFromARGB16Buffer(const Buffer& b, int tWidth, int tHeight)
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

TextureData loadTextureFromTwiddledARGB16Buffer(const Buffer& b, int tWidth, int tHeight) {
	Buffer untwiddled = untwiddleBuffer(b, (uint32_t)tWidth, (uint32_t)tHeight);
	TextureData ret = loadTextureFromARGB16Buffer(untwiddled, tWidth, tHeight);
	freeBuffer(untwiddled);
	return ret;
}

TextureData loadTextureFromARGB32Buffer(const Buffer& b, int tWidth, int tHeight) {
	uint32_t amask = 0xff000000;
	uint32_t rmask = 0x00ff0000;
	uint32_t gmask = 0x0000ff00;
	uint32_t bmask = 0x000000ff;
	
	int depth = 32;
	int pitch = 4 * tWidth;

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(b.mData, tWidth, tHeight, depth, pitch, rmask, gmask, bmask, amask);
	return textureFromSurface(surface);
}

TextureData loadTextureFromRawPNGBuffer(const Buffer& b, int tWidth, int tHeight) {
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

TextureData loadPalettedTextureFrom8BitBuffer(const Buffer& b, int tPaletteID, int tWidth, int tHeight) {
	TextureData returnData;
	returnData.mTexture = allocTextureMemory(sizeof(SDLTextureData));
	returnData.mTextureSize.x = tWidth;
	returnData.mTextureSize.y = tHeight;
	returnData.mHasPalette = 1;
	returnData.mPaletteID = tPaletteID;
	Texture texture = (Texture)returnData.mTexture->mData;

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &texture->mTexture);
	glBindTexture(GL_TEXTURE_2D, texture->mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tWidth, tHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, b.mData);
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return returnData;
}

#ifdef _WIN32
#define Rectangle Rectangle2
#include <Windows.h>
#undef Rectangle
// Get system font file path (Taken from https://stackoverflow.com/questions/11387564/get-a-font-filepath-from-name-and-style-in-c-windows)
static std::string getSystemFontFile(const std::string& tFaceName) {

	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	std::wstring wsFaceName(tFaceName.begin(), tFaceName.end());

	// Open Windows font registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		return "";
	}

	DWORD maxValueNameSize, maxValueDataSize;
	result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
	if (result != ERROR_SUCCESS) {
		return "";
	}

	DWORD valueIndex = 0;
	std::vector<WCHAR> valueName = std::vector<WCHAR>(maxValueNameSize);
	std::vector<BYTE> valueData = std::vector<BYTE>(maxValueDataSize);
	DWORD valueNameSize, valueDataSize, valueType;
	std::wstring wsFontFile;

	// Look for a matching font name
	do {
		wsFontFile.clear();
		valueDataSize = maxValueDataSize;
		valueNameSize = maxValueNameSize;

		result = RegEnumValue(hKey, valueIndex, valueName.data(), &valueNameSize, 0, &valueType, valueData.data(), &valueDataSize);

		valueIndex++;

		if (result != ERROR_SUCCESS || valueType != REG_SZ) {
			continue;
		}

		std::wstring wsValueName(valueName.data(), valueNameSize);

		// Found a match
		if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.length()) == 0) {

			wsFontFile.assign((LPWSTR)valueData.data(), valueDataSize);
			break;
		}
	} while (result != ERROR_NO_MORE_ITEMS);

	RegCloseKey(hKey);

	if (wsFontFile.empty()) {
		return "";
	}

	// Build full font file path
	WCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir, MAX_PATH);

	std::wstringstream ss;
	ss << winDir << "\\Fonts\\" << wsFontFile;
	wsFontFile = ss.str();

#pragma warning( push )
#pragma warning( disable : 4244 ) // implicit conversion from wstring element to string element, but it's intended
	return std::string(wsFontFile.begin(), wsFontFile.end());
#pragma warning( pop )
}
#else 
static std::string getSystemFontFile(const std::string &tFaceName) {
	return "C:/Windows/Fonts/" + tFaceName;
}
#endif

TruetypeFont loadTruetypeFont(const char *, double)
{
	return nullptr;
}

void unloadTruetypeFont(TruetypeFont)
{

}

typedef unsigned char BYTE;

void saveScreenShot(const char* tFileDir) {
	const auto sz = getDisplayedScreenSize();
	std::vector<BYTE> pixels(3 * sz.x * sz.y);
	glReadPixels(0, 0, sz.x, sz.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	saveRGB32ToPNG(makeBuffer(pixels.data(), pixels.size()), sz.x, sz.y, tFileDir);
}