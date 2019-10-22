#include "prism/texture.h"

#include <assert.h>

#include <kos.h>
#include <kos/string.h>
#include <png/png.h>

#include "prism/file.h"

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/compression.h"

#define HEADER_SIZE_KMG 64
// TODO: use kmg.h from KOS

extern semaphore_t gPVRAccessSemaphore;

typedef struct {
	BufferPointer p;
	Buffer b;
} PNGReadCaller;

static void readPNGDataFromInputStream(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
	png_voidp io_ptr = png_get_io_ptr(png_ptr);
	if (io_ptr == NULL) {
		logError("Did not get caller");
		recoverFromError();
	}

	PNGReadCaller* caller = (PNGReadCaller*)io_ptr;
	if (((uint32_t)caller->p) + byteCountToRead > ((uint32_t)caller->b.mData) + caller->b.mLength) {
		logError("Trying to read outside buffer");
		recoverFromError();
	}
	readFromBufferPointer(outBytes, &caller->p, byteCountToRead);
}

TextureData loadTexturePNG(const char* tFileDir) {
	Buffer rawPNGBuffer = fileToBuffer(tFileDir);
	BufferPointer p = getBufferPointer(rawPNGBuffer);

	uint8_t* pngSignature = (uint8_t*)p;
	p += 8;
	if (!png_check_sig(pngSignature, 8)) {
		logError("Invalid png signature");
		recoverFromError();
	}

	png_structp png_ptr = NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL) {
		logError("Cannot create read struct.");
		recoverFromError();
	}

	png_infop info_ptr = NULL;
	info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		logError("Cannot create info struct.");
		recoverFromError();
	}

	PNGReadCaller caller;
	caller.p = p;
	caller.b = rawPNGBuffer;

	png_set_read_fn(png_ptr, &caller, readPNGDataFromInputStream);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bitDepth = 0;
	int colorType = -1;
	png_uint_32 retval = png_get_IHDR(png_ptr, info_ptr,
		&width,
		&height,
		&bitDepth,
		&colorType,
		NULL, NULL, NULL);

	if (retval != 1) {
		logError("Unable to read image data");
		recoverFromError();
	}

	int hasAlpha = 0;
	if (colorType == PNG_COLOR_TYPE_RGB) {
		hasAlpha = 0;
	}
	else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
		hasAlpha = 1;
	}
	else {
		logError("Unsupported color type");
		logErrorInteger(colorType);
		recoverFromError();
	}

	const size_t length = width * height * 4;
	uint8_t* dst = (uint8_t*)allocMemory(width*height * 4);

	const png_uint_32 bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
	char* rowData = (char*)allocMemory(bytesPerRow);

	// read single row at a time
	uint32_t rowIdx;
	for (rowIdx = 0; rowIdx < (uint32_t)height; ++rowIdx)
	{
		png_read_row(png_ptr, (png_bytep)rowData, NULL);

		uint32_t rowOffset = rowIdx * width;

		uint32_t byteIndex = 0;
		uint32_t colIdx;
		for (colIdx = 0; colIdx < (uint32_t)width; ++colIdx)
		{
			uint32_t targetPixelIndex = rowOffset + colIdx;
			dst[targetPixelIndex * 4 + 2] = rowData[byteIndex++];
			dst[targetPixelIndex * 4 + 1] = rowData[byteIndex++];
			dst[targetPixelIndex * 4 + 0] = rowData[byteIndex++];
			dst[targetPixelIndex * 4 + 3] = hasAlpha ? rowData[byteIndex++] : 255;
		}
		assert(byteIndex == bytesPerRow);
	}

	freeMemory(rowData);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	freeBuffer(rawPNGBuffer);

	Buffer b = makeBufferOwned(dst, length);
	auto ret = loadTextureFromARGB32Buffer(b, int(width), int(height));
	freeBuffer(b);
	return ret;
}

TextureData loadTexturePKG(const char* tFileDir) {

  TextureData returnData;
  returnData.mHasPalette = 0;

  auto pkgBuffer = fileToBuffer(tFileDir);
  decompressBufferZSTD(&pkgBuffer);
  char* kmgData = (char*)pkgBuffer.mData;
  size_t bufferLength = pkgBuffer.mLength;

  returnData.mTextureSize.x = 0;
  returnData.mTextureSize.y = 0;

  memcpy4(&returnData.mTextureSize.x, kmgData + 16, sizeof returnData.mTextureSize.x);
  memcpy4(&returnData.mTextureSize.y, kmgData + 20, sizeof returnData.mTextureSize.y);

  sem_wait(&gPVRAccessSemaphore);
  returnData.mTexture = allocTextureMemory(bufferLength - HEADER_SIZE_KMG);
  sq_cpy(returnData.mTexture->mData, kmgData + HEADER_SIZE_KMG, bufferLength - HEADER_SIZE_KMG);
  sem_signal(&gPVRAccessSemaphore);

  freeBuffer(pkgBuffer);

  return returnData;
}

TextureData loadTexture(const char* tFileDir) {
	const char* fileExt = getFileExtension(tFileDir);

	if(!strcmp("pkg", fileExt)) {
		return loadTexturePKG(tFileDir);
	} else if (!strcmp("png", fileExt)) {
		return loadTexturePNG(tFileDir);
	}
	else {
		logError("Unable to identify texture file type.");
		logErrorString(fileExt);
		abortSystem();
		TextureData errData;
		memset(&errData, 0, sizeof errData);
		return errData;
	}
}

TextureData loadTextureFromARGB16Buffer(Buffer b, int tWidth, int tHeight) {
	Buffer twiddledBuffer = twiddleTextureBuffer16(b, tWidth, tHeight);
	TextureData ret = loadTextureFromTwiddledARGB16Buffer(twiddledBuffer, tWidth, tHeight);
	freeBuffer(twiddledBuffer);
	return ret;
}

TextureData loadTextureFromTwiddledARGB16Buffer(Buffer b, int tWidth, int tHeight) {
	TextureData returnData;
	returnData.mHasPalette = 0;
	returnData.mTextureSize.x = tWidth;
	returnData.mTextureSize.y = tHeight;

    sem_wait(&gPVRAccessSemaphore);
 	returnData.mTexture = allocTextureMemory(b.mLength);
  	sq_cpy(returnData.mTexture->mData, b.mData, b.mLength);
	sem_signal(&gPVRAccessSemaphore);

	return returnData;
}

TextureData loadTextureFromARGB32Buffer(Buffer b, int tWidth, int tHeight) {
	Buffer argb16Buffer = turnARGB32BufferIntoARGB16Buffer(b);
	Buffer twiddledBuffer = twiddleTextureBuffer16(argb16Buffer, tWidth, tHeight);	
	freeBuffer(argb16Buffer);

	TextureData returnData;
	returnData.mHasPalette = 0;
	returnData.mTextureSize.x = tWidth;
	returnData.mTextureSize.y = tHeight;

        sem_wait(&gPVRAccessSemaphore);
 	returnData.mTexture = allocTextureMemory(twiddledBuffer.mLength);
  	sq_cpy(returnData.mTexture->mData, twiddledBuffer.mData, twiddledBuffer.mLength);
        sem_signal(&gPVRAccessSemaphore);

	freeBuffer(twiddledBuffer);
	return returnData;
} 

TextureData loadPalettedTextureFrom8BitBuffer(Buffer b, int tPaletteID, int tWidth, int tHeight) {
	Buffer twiddledBuffer = twiddleTextureBuffer8(b, tWidth, tHeight);

	TextureData returnData;
	returnData.mHasPalette = 1;
	returnData.mPaletteID = tPaletteID;
	returnData.mTextureSize.x = tWidth;
	returnData.mTextureSize.y = tHeight;
	
        sem_wait(&gPVRAccessSemaphore);
	returnData.mTexture = allocTextureMemory(twiddledBuffer.mLength);
  	sq_cpy(returnData.mTexture->mData, twiddledBuffer.mData, twiddledBuffer.mLength);
        sem_signal(&gPVRAccessSemaphore);

	freeBuffer(twiddledBuffer);
	return returnData;
}

void unloadTexture(TextureData tTexture) {
  freeTextureMemory(tTexture.mTexture);
}

int getTextureHash(TextureData tTexture) {
	return (int)tTexture.mTexture;
}

int canLoadTexture(const char* tPath) {
	const char* fileExt = getFileExtension(tPath);

	return (!strcmp("pkg", fileExt) && isFile(tPath));
}

TruetypeFont loadTruetypeFont(const char* tName, double tSize) {
	(void) tName;
	(void) tSize;

	return NULL; // TODO
}

void unloadTruetypeFont(TruetypeFont tFont) {
	(void) tFont; // TODO
}

void saveScreenShot(const char* tFileDir) {
	char fullPath[1024];
	getFullPath(fullPath, tFileDir);
	char ppmPath[1024];
	char* fileExtension = strrchr(fullPath, '.');
	if (fileExtension) *fileExtension = '\0';
	sprintf(ppmPath, "%s.ppm", fullPath);

	vid_screen_shot(ppmPath);
}