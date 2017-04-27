#include "tari/texture.h"

#include <kos.h>
#include <kos/string.h>

// TODO: remove quicklz and remove it with something suitable
#include "tari/quicklz.h"

#include "tari/file.h"

#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"

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
		return errData;
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

	return (!strcmp("pkg", fileExt) && isFile(tPath));
}
