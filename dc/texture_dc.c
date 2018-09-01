#include "prism/texture.h"

#include <assert.h>

#include <kos.h>
#include <kos/string.h>

// TODO: remove quicklz and remove it with something suitable
#include "prism/quicklz.h"

#include "prism/file.h"

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/math.h"

#define HEADER_SIZE_KMG 64
// TODO: use kmg.h from KOS

extern semaphore_t gPVRAccessSemaphore;

TextureData loadTexturePKG(char* tFileDir) {

  TextureData returnData;
  returnData.mHasPalette = 0;

  qlz_state_decompress *state_decompress = (qlz_state_decompress *) allocMemory(sizeof(qlz_state_decompress));
  size_t bufferLength;
  char* kmgData;
  Buffer pkgBuffer;

  pkgBuffer = fileToBuffer(tFileDir);

  bufferLength = qlz_size_decompressed(pkgBuffer.mData);
  debugInteger(bufferLength);

  kmgData = (char*) allocMemory(bufferLength);

  // decompress and write result
  bufferLength = qlz_decompress(pkgBuffer.mData, kmgData, state_decompress);
  debugInteger(bufferLength);

  freeBuffer(pkgBuffer);
  freeMemory(state_decompress);

  returnData.mTextureSize.x = 0;
  returnData.mTextureSize.y = 0;

  memcpy4(&returnData.mTextureSize.x, kmgData + 16, sizeof returnData.mTextureSize.x);
  memcpy4(&returnData.mTextureSize.y, kmgData + 20, sizeof returnData.mTextureSize.y);

  sem_wait(&gPVRAccessSemaphore);
  returnData.mTexture = allocTextureMemory(bufferLength - HEADER_SIZE_KMG);
  sq_cpy(returnData.mTexture->mData, kmgData + HEADER_SIZE_KMG, bufferLength - HEADER_SIZE_KMG);
  sem_signal(&gPVRAccessSemaphore);

  freeMemory(kmgData);

  return returnData;
}

TextureData loadTexture(char* tFileDir) {
	char* fileExt = getFileExtension(tFileDir);

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

/* Linear/iterative twiddling algorithm from Marcus' tatest */
#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )
#define MIN(a, b) ( (a)<(b)? (a):(b) )

/* This twiddling code is copied from pvr_texture.c, and the original
   algorithm was written by Vincent Penne. */

static Buffer twiddleTextureBuffer16(Buffer tBuffer, int tWidth, int tHeight) {
    int w = tWidth;
    int h = tHeight;
    int mini = min(w, h);
    int mask = mini - 1;
    uint16 * pixels = (uint16 *)tBuffer.mData;
    uint16 * vtex = allocMemory(tBuffer.mLength);
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

static Buffer twiddleTextureBuffer8(Buffer tBuffer, int tWidth, int tHeight) {
    int w = tWidth;
    int h = tHeight;
    int mini = min(w, h);
    int mask = mini - 1;
    uint8 * pixels = (uint8 *)tBuffer.mData;
    uint8 * vtex = allocMemory(tBuffer.mLength);
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

TextureData loadTextureFromARGB16Buffer(Buffer b, int tWidth, int tHeight) {
	Buffer twiddledBuffer = twiddleTextureBuffer16(b, tWidth, tHeight);	

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

int canLoadTexture(char* tPath) {
	char* fileExt = getFileExtension(tPath);

	return (!strcmp("pkg", fileExt) && isFile(tPath));
}

TruetypeFont loadTruetypeFont(char* tName, double tSize) {
	(void) tName;
	(void) tSize;

	return NULL; // TODO
}

void unloadTruetypeFont(TruetypeFont tFont) {
	(void) tFont; // TODO
}
