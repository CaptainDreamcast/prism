#pragma once

#include "file.h"

namespace prism {

void compressBufferZSTD(Buffer* tBuffer);
void decompressBufferZSTD(Buffer* tBuffer);

}