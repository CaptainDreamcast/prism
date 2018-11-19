#pragma once

#include "file.h"

void compressBuffer(Buffer* tBuffer);
void decompressBuffer(Buffer* tBuffer);

void compressBufferZSTD(Buffer* tBuffer);
void decompressBufferZSTD(Buffer* tBuffer);
