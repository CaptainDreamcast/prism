#pragma once
/* Minimal zstd.h shim for environments that have the libzstd runtime
 * (libzstd.so.1) but not the dev package. Declares only the functions
 * prism uses (compression.cpp, memoryhandler.cpp). If the real zstd.h
 * is available, prefer it by putting its include dir earlier on the
 * include path (or just don't add this shim dir). ABI-stable API. */

#include <stddef.h>

#if defined (__cplusplus)
extern "C" {
#endif

size_t ZSTD_compress(void* dst, size_t dstCapacity,
                     const void* src, size_t srcSize,
                     int compressionLevel);
size_t ZSTD_decompress(void* dst, size_t dstCapacity,
                       const void* src, size_t compressedSize);

#define ZSTD_CONTENTSIZE_UNKNOWN (0ULL - 1)
#define ZSTD_CONTENTSIZE_ERROR   (0ULL - 2)
unsigned long long ZSTD_getFrameContentSize(const void* src, size_t srcSize);

size_t ZSTD_compressBound(size_t srcSize);
unsigned ZSTD_isError(size_t code);

#if defined (__cplusplus)
}
#endif
