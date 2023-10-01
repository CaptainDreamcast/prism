#include "prism/compression.h"

#include <zstd.h>

#include "prism/log.h"
#include "prism/system.h"
#include "prism/memoryhandler.h"

static const int COMPRESSION_BUFFER = 400;

void compressBufferZSTD(Buffer * tBuffer)
{
	if (!tBuffer->mIsOwned) {
		logError("Unable to compress unowned Buffer");
		recoverFromError();
	}

	char* src = (char*)tBuffer->mData;
	auto dstBufferSize = size_t(tBuffer->mLength + COMPRESSION_BUFFER);
	char* dst = (char*)allocMemory(int(dstBufferSize));
	auto dstLength = ZSTD_compress(dst, dstBufferSize, src, tBuffer->mLength, 1);
	dst = (char*)reallocMemory(dst, int(dstLength));

	freeBuffer(*tBuffer);
	*tBuffer = makeBufferOwned(dst, int(dstLength));
}

void decompressBufferZSTD(Buffer * tBuffer)
{
	if (!tBuffer->mIsOwned) {
		logError("Unable to decompress unowned Buffer");
		recoverFromError();
	}

	char* src = (char*)tBuffer->mData;
	size_t uncompressedLength = (size_t)ZSTD_getFrameContentSize(src, tBuffer->mLength);

	char* dst = (char*)allocMemory(int(uncompressedLength));
	auto dstLength = ZSTD_decompress(dst, uncompressedLength, src, size_t(tBuffer->mLength));
	dst = (char*)reallocMemory(dst, int(dstLength));

	freeBuffer(*tBuffer);
	*tBuffer = makeBufferOwned(dst, uint32_t(dstLength));
}
