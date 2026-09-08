#include "prism/texture.h"

#include <vita2d.h>

#include <string.h>
#include <sstream>
#include <vector>
#include <cassert>

#include "prism/file.h"
#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/compression.h"

#include <psp2/kernel/sysmem.h>

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

	static TextureData loadTexturePKGVita(const char* tFileDir) {
		Buffer b = fileToBuffer(tFileDir);
		decompressBufferZSTD(&b);
		KMGHeader hdr = untwiddleKMGBufferAndReturnHeader(&b);
		TextureData ret = loadTextureFromARGB16Buffer(b, hdr.width, hdr.height);
		freeBuffer(b);
		return ret;
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
			return loadTexturePKGVita(fullFileName);
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

	// vita2d gives every texture its own memblock and its own sceGxmMapMemory. 
	// If we don't use these slabs, we might run out of GPU mappings, which caused failures with lots of sprites
	static const uint32_t VITA_TEXTURE_SLAB_SIZE = 8 * 1024 * 1024;
	static const uint32_t VITA_TEXTURE_SLAB_ALIGNMENT = 64;

	typedef struct {
		SceUID mUID;
		uint8_t* mBase;
		uint32_t mSize;
		uint32_t mOffset;
		int mLiveTextureAmount;
	} VitaTextureSlab;

	static struct {
		int mCreatedTextureAmount;
		int mFreedPaletteAmount;
		std::vector<VitaTextureSlab> mSlabs;
	} gPrismVitaTextureData;

	static void freeUnusedVita2dPalette(vita2d_texture* tTexture);

	static vita2d_texture* createVitaTextureFromSlab(int tWidth, int tHeight, SceGxmTextureFormat tFormat);

	static vita2d_texture* createVitaTextureOrAbort(int tWidth, int tHeight, SceGxmTextureFormat tFormat) {
		auto ret = createVitaTextureFromSlab(tWidth, tHeight, tFormat);
		if (!ret) {
			logErrorFormat("[Texture] Out of Vita texture memory creating %dx%d texture (format %d) after %d textures.", tWidth, tHeight, (int)tFormat, gPrismVitaTextureData.mCreatedTextureAmount);
			abortSystem();
		}
		freeUnusedVita2dPalette(ret);
		return ret;
	}

	static int getBytesPerPixelForVitaTextureFormat(SceGxmTextureFormat tFormat) {
		switch (tFormat & 0x9f000000U) {
		case SCE_GXM_TEXTURE_BASE_FORMAT_U8:
		case SCE_GXM_TEXTURE_BASE_FORMAT_P8:
			return 1;
		case SCE_GXM_TEXTURE_BASE_FORMAT_U4U4U4U4:
		case SCE_GXM_TEXTURE_BASE_FORMAT_U1U5U5U5:
		case SCE_GXM_TEXTURE_BASE_FORMAT_U5U6U5:
			return 2;
		default:
			return 4;
		}
	}

	static void logVitaTextureSlabAllocation(uint32_t tSlabSize) {
		SceKernelFreeMemorySizeInfo info;
		info.size = sizeof(info);
		if (sceKernelGetFreeMemorySize(&info) < 0) return;
		debugFormat("[Texture] Texture slab %d allocated (%d bytes, free cdram %d, user %d)", (int)gPrismVitaTextureData.mSlabs.size(), (int)tSlabSize, info.size_cdram, info.size_user);
	}

	static uint8_t* allocateFromTextureSlabsOrNull(uint32_t tSize) {
		const auto alignedSize = (tSize + VITA_TEXTURE_SLAB_ALIGNMENT - 1) & ~(VITA_TEXTURE_SLAB_ALIGNMENT - 1);
		for (auto& slab : gPrismVitaTextureData.mSlabs) {
			if (slab.mOffset + alignedSize > slab.mSize) continue;
			uint8_t* ret = slab.mBase + slab.mOffset;
			slab.mOffset += alignedSize;
			slab.mLiveTextureAmount++;
			return ret;
		}

		VitaTextureSlab slab;
		const auto slabSize = (alignedSize > VITA_TEXTURE_SLAB_SIZE) ? alignedSize : VITA_TEXTURE_SLAB_SIZE;
		// CDRAM first: it is the faster pool for GPU texture reads, and vitaGpuAlloc falls back to main memory when it fills.
		slab.mBase = (uint8_t*)vitaGpuAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, slabSize, VITA_TEXTURE_SLAB_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE, &slab.mUID);
		if (!slab.mBase) return NULL;
		slab.mSize = slabSize;
		slab.mOffset = alignedSize;
		slab.mLiveTextureAmount = 1;
		gPrismVitaTextureData.mSlabs.push_back(slab);
		logVitaTextureSlabAllocation(slabSize);
		return slab.mBase;
	}

	// sceGxmTextureInitLinear rejects dimensions above this and leaves the texture unusable, so don't try to allocate them
	static const int VITA_MAXIMUM_TEXTURE_DIMENSION = 4096;

	static vita2d_texture* createVitaTextureFromSlab(int tWidth, int tHeight, SceGxmTextureFormat tFormat) {
		if (tWidth > VITA_MAXIMUM_TEXTURE_DIMENSION || tHeight > VITA_MAXIMUM_TEXTURE_DIMENSION) return NULL;

		const uint32_t stride = (uint32_t(tWidth) + 7) & ~7u;
		const uint32_t textureSize = stride * uint32_t(tHeight) * uint32_t(getBytesPerPixelForVitaTextureFormat(tFormat));
		uint8_t* data = allocateFromTextureSlabsOrNull(textureSize);
		if (!data) return NULL;

		auto ret = (vita2d_texture*)malloc(sizeof(vita2d_texture));
		if (!ret) return NULL;
		memset(data, 0, textureSize);
		sceGxmTextureInitLinear(&ret->gxm_tex, data, tFormat, tWidth, tHeight, 0);
		ret->data_UID = 0;
		ret->palette_UID = 0;
		ret->depth_UID = 0;
		ret->gxm_rtgt = 0;
		return ret;
	}

	// Textures whose data came from a slab carry data_UID 0, so vita2d_free_texture leaves the pixels alone; the slab itself goes back once its last texture is released 
	// Textures vita2d allocated itself (the PNG paths) are not in any slab and are left untouched
	void releaseVitaTextureDataFromSlab(vita2d_texture* tTexture) {
		if (!tTexture) return;
		const uint8_t* data = (const uint8_t*)vita2d_texture_get_datap(tTexture);
		for (size_t i = 0; i < gPrismVitaTextureData.mSlabs.size(); i++) {
			auto& slab = gPrismVitaTextureData.mSlabs[i];
			if (data < slab.mBase || data >= slab.mBase + slab.mSize) continue;

			slab.mLiveTextureAmount--;
			if (!slab.mLiveTextureAmount) {
				vitaGpuFree(slab.mUID);
				gPrismVitaTextureData.mSlabs.erase(gPrismVitaTextureData.mSlabs.begin() + i);
			}
			return;
		}
	}

	static void freeUnusedVita2dPalette(vita2d_texture* tTexture) {
		if (!tTexture->palette_UID) return;

		vitaGpuFree(tTexture->palette_UID);
		tTexture->palette_UID = 0;
		gPrismVitaTextureData.mFreedPaletteAmount++;
	}

	TextureData loadTextureFromARGB16Buffer(const Buffer& b, int tWidth, int tHeight)
	{
		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = tWidth;
		returnData.mTextureSize.y = tHeight;
		returnData.mHasPalette = 0;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = createVitaTextureOrAbort(tWidth, tHeight, SCE_GXM_TEXTURE_FORMAT_A4R4G4B4);

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
		texture->mTexture = createVitaTextureOrAbort(tWidth, tHeight, SCE_GXM_TEXTURE_FORMAT_A8R8G8B8);

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
		if (!vitaTexture) {
			logErrorFormat("[Texture] Out of Vita texture memory loading PNG buffer of %d bytes after %d live memory blocks.", (int)b.mLength, getAllocatedMemoryBlockAmount());
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

	TextureData loadPalettedTextureFrom8BitBuffer(const Buffer& b, int tPaletteID, int tWidth, int tHeight) {
		TextureData returnData;
		returnData.mTexture = allocTextureMemory(sizeof(VitaTextureData));
		returnData.mTextureSize.x = tWidth;
		returnData.mTextureSize.y = tHeight;
		returnData.mHasPalette = 1;
		returnData.mPaletteID = tPaletteID;
		Texture texture = (Texture)returnData.mTexture->mData;
		texture->mTexture = createVitaTextureOrAbort(tWidth, tHeight, SCE_GXM_TEXTURE_FORMAT_P8_RGBA);

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

	TruetypeFont loadTruetypeFont(const char*, float)
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

	void copyScreenShotToClipboard() {
		// UNSUPPORTED
	}
}