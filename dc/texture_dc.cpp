#include "prism/texture.h"

#include <assert.h>

#include <kos.h>
#include <kos/string.h>

#include "prism/file.h"

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"
#include "prism/compression.h"

#define HEADER_SIZE_KMG 64
// TODO: use kmg.h from KOS

extern semaphore_t gPVRAccessSemaphore;

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
	} else {
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
