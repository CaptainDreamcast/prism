#include "prism/texture.h"

#include <vita2d.h>

#include <string.h>
#include <sstream>
#include <cassert>

#include "prism/file.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/compression.h"

namespace prism {

	TextureData loadTexturePNG(const char* tFileDir) {

		Buffer b = fileToBuffer(tFileDir);
		auto vitaTexture = vita2d_load_PNG_buffer(b.mData);
		freeBuffer(b);
		if (vitaTexture == NULL)
		{
			logError("Unable to load file:");
			logErrorString(tFileDir);
			abortSystem();
		}

		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = vita2d_texture_get_width(vitaTexture);
		returnData.mTextureSize.y = vita2d_texture_get_height(vitaTexture);
		returnData.mHasPalette = 0;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = vitaTexture;

		return returnData;
	}

#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )
#define MIN(a, b) ( (a)<(b)? (a):(b) )


	static void untwiddle(uint16_t* src, uint16_t* output, int w, int h) {
		int min = MIN(w, h);
		int mask = min - 1;
		uint16_t* pixels = output;
		uint16_t* vtex = src;
		int x, y, yout;

		for (y = 0; y < h; y++) {
			yout = y;

			for (x = 0; x < w; x++) {
				pixels[y * w + x] = vtex[TWIDOUT(x & mask, yout & mask) +
					(x / min + yout / min) * min * min];
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
		assert(false);
		return TextureData();
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
		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = tWidth;
		returnData.mTextureSize.y = tHeight;
		returnData.mHasPalette = 0;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = vita2d_create_empty_texture_format(tWidth, tHeight, SCE_GXM_TEXTURE_FORMAT_A4R4G4B4);

		auto data = vita2d_texture_get_datap(texture->mTexture);
		auto textureStride = vita2d_texture_get_stride(texture->mTexture);
		auto bufferStride = tWidth * sizeof(uint16_t);
		for (int y = 0; y < tHeight; y++)
		{
			auto targetMem = ((char*)data) + (textureStride * y);
			auto sourceMem = ((char*)b.mData) + (bufferStride * y);
			memcpy(targetMem, sourceMem, bufferStride);
		}

		texture->mTexture->palette_UID = 0;
		texture->mTexture->depth_UID = 0; // not zeroed in vita2d apparently
		texture->mTexture->gxm_rtgt = 0;

		return returnData;
	}

	TextureData loadTextureFromTwiddledARGB16Buffer(const Buffer& b, int tWidth, int tHeight) {
		Buffer untwiddled = untwiddleBuffer(b, (uint32_t)tWidth, (uint32_t)tHeight);
		TextureData ret = loadTextureFromARGB16Buffer(untwiddled, tWidth, tHeight);
		freeBuffer(untwiddled);
		return ret;
	}

	TextureData loadTextureFromARGB32Buffer(const Buffer& b, int tWidth, int tHeight) {
		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = tWidth;
		returnData.mTextureSize.y = tHeight;
		returnData.mHasPalette = 0;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = vita2d_create_empty_texture_format(tWidth, tHeight, SCE_GXM_TEXTURE_FORMAT_A8R8G8B8);

		auto data = vita2d_texture_get_datap(texture->mTexture);
		auto textureStride = vita2d_texture_get_stride(texture->mTexture);
		auto bufferStride = tWidth * sizeof(uint32_t);
		for (int y = 0; y < tHeight; y++)
		{
			auto targetMem = ((char*)data) + (textureStride * y);
			auto sourceMem = ((char*)b.mData) + (bufferStride * y);
			memcpy(targetMem, sourceMem, bufferStride);	
		}

		texture->mTexture->palette_UID = 0;
		texture->mTexture->depth_UID = 0; // not zeroed in vita2d apparently
		texture->mTexture->gxm_rtgt = 0;

		return returnData;
	}

	TextureData loadTextureFromRawPNGBuffer(const Buffer& b, int tWidth, int tHeight) {
		(void)tWidth;
		(void)tHeight;
		auto vitaTexture = vita2d_load_PNG_buffer(b.mData);
		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = vita2d_texture_get_width(vitaTexture);
		returnData.mTextureSize.y = vita2d_texture_get_height(vitaTexture);
		returnData.mHasPalette = 0;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = vitaTexture;
		return returnData;
	}

	TextureData loadPalettedTextureFrom8BitBuffer(const Buffer& b, int tPaletteID, int tWidth, int tHeight) {
		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = tWidth;
		returnData.mTextureSize.y = tHeight;
		returnData.mHasPalette = 1;
		returnData.mPaletteID = tPaletteID;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = vita2d_create_empty_texture_format(tWidth, tHeight, SCE_GXM_TEXTURE_FORMAT_P8_RGBA);

		auto data = vita2d_texture_get_datap(texture->mTexture);
		auto textureStride = vita2d_texture_get_stride(texture->mTexture);
		auto bufferStride = tWidth;
		for (int y = 0; y < tHeight; y++)
		{
			auto targetMem = ((char*)data) + (textureStride * y);
			auto sourceMem = ((char*)b.mData) + (bufferStride * y);
			memcpy(targetMem, sourceMem, bufferStride);
		}
		
		texture->mTexture->palette_UID = 0;
		texture->mTexture->depth_UID = 0; // not zeroed by default in vita2d apparently
		texture->mTexture->gxm_rtgt = 0;
		
		return returnData;
	}

	static std::string getSystemFontFile(const std::string& tFaceName) {
		return "C:/Windows/Fonts/" + tFaceName;
	}

	TruetypeFont loadTruetypeFont(const char*, double)
	{
		return nullptr;
	}

	void unloadTruetypeFont(TruetypeFont)
	{

	}

	typedef unsigned char BYTE;

	void saveScreenShot(const char* tFileDir) {
		// UNSUPPORTED
	}
}