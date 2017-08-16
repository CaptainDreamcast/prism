#include "tari/texture.h"

#include <assert.h>

#include <kos.h>
#include <kos/string.h>

// TODO: remove quicklz and remove it with something suitable
#include "tari/quicklz.h"

#include "tari/file.h"

#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"
#include "tari/math.h"

#define HEADER_SIZE_KMG 64
// TODO: use kmg.h from KOS

TextureData loadTexturePKG(char* tFileDir) {

  TextureData returnData;

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

  returnData.mTexture = allocTextureMemory(bufferLength - HEADER_SIZE_KMG);
  sq_cpy(returnData.mTexture->mData, kmgData + HEADER_SIZE_KMG, bufferLength - HEADER_SIZE_KMG);

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

static Buffer turnARGB32BufferIntoARGB16Buffer(Buffer tSrc, int tWidth, int tHeight) {
	int dstSize = tWidth*tHeight*2;
	char* dst = allocMemory(dstSize);
	char* src = tSrc.mData;
	assert((int)tSrc.mLength >= dstSize*2);

	int i;
	for(i = 0; i < dstSize; i++) {
		int srcPos = 2*i;
		int dstPos = i;

		uint8_t val = ((uint8_t)src[srcPos]) / 2;
		dst[dstPos] = val;

		val = ((uint8_t)src[srcPos+1]) / 2;
		dst[dstPos] |= (val << 4);
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

static Buffer twiddleTextureBuffer(Buffer tBuffer, int tWidth, int tHeight) {
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

TextureData loadTextureFromARGB32Buffer(Buffer b, int tWidth, int tHeight) {
	Buffer argb16Buffer = turnARGB32BufferIntoARGB16Buffer(b, tWidth, tHeight);
	Buffer twiddledBuffer = twiddleTextureBuffer(argb16Buffer, tWidth, tHeight);	
	freeBuffer(argb16Buffer);

	TextureData returnData;
	returnData.mTextureSize.x = tWidth;
	returnData.mTextureSize.y = tHeight;

 	returnData.mTexture = allocTextureMemory(twiddledBuffer.mLength);
  	sq_cpy(returnData.mTexture->mData, twiddledBuffer.mData, twiddledBuffer.mLength);

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