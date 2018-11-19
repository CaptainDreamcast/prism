#include "prism/compression.h"

#include <zstd.h>

#include "prism/quicklz.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/memoryhandler.h"

static const int COMPRESSION_BUFFER = 400;


void compressBuffer(Buffer* tBuffer) {
	if (!tBuffer->mIsOwned) {
		logError("Unable to compress unowned Buffer");
		recoverFromError();
	}

	Buffer src = *tBuffer;
	Buffer dst = *tBuffer;

	qlz_state_compress state_compress;

	dst.mData = allocMemory(src.mLength + COMPRESSION_BUFFER);
	dst.mLength = qlz_compress(src.mData, dst.mData, src.mLength, &state_compress);
	dst.mData = reallocMemory(dst.mData, dst.mLength);
	dst.mIsOwned = 1;

	freeBuffer(src);

	*tBuffer = dst;
}

void decompressBuffer(Buffer * tBuffer)
{
	if (!tBuffer->mIsOwned) {
		logError("Unable to decompress unowned Buffer");
		recoverFromError();
	}

	Buffer src = *tBuffer;
	Buffer dst = *tBuffer;

	qlz_state_decompress state_decompress;
	size_t len = qlz_size_decompressed(src.mData);

	dst.mData = allocMemory(len);
	dst.mLength = qlz_decompress(src.mData, dst.mData, &state_decompress);
	dst.mData = reallocMemory(dst.mData, dst.mLength);
	dst.mIsOwned = 1;

	freeBuffer(src);
	*tBuffer = dst;
}

void compressBufferZSTD(Buffer * tBuffer)
{
	if (!tBuffer->mIsOwned) {
		logError("Unable to compress unowned Buffer");
		recoverFromError();
	}

	char* src = tBuffer->mData;
	int dstBufferSize = tBuffer->mLength + COMPRESSION_BUFFER;
	char* dst = malloc(dstBufferSize);
	int dstLength = ZSTD_compress(dst, dstBufferSize, src, tBuffer->mLength, 1);
	dst = realloc(dst, dstLength);

	freeBuffer(*tBuffer);
	*tBuffer = makeBufferOwned(dst, dstLength);
}

void decompressBufferZSTD(Buffer * tBuffer)
{
	if (!tBuffer->mIsOwned) {
		logError("Unable to decompress unowned Buffer");
		recoverFromError();
	}

	char* src = tBuffer->mData;
	size_t uncompressedLength = (size_t)ZSTD_getFrameContentSize(src, tBuffer->mLength);

	char* dst = malloc(uncompressedLength);
	int dstLength = ZSTD_decompress(dst, uncompressedLength, src, tBuffer->mLength);
	dst = realloc(dst, dstLength);

	freeBuffer(*tBuffer);
	*tBuffer = makeBufferOwned(dst, dstLength);
}
