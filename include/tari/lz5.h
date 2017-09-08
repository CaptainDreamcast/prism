#pragma once

#include <stdint.h>

#include "common/header.h"

fup void decompressLZ5(uint8_t* tDst, uint8_t* tSrc, uint32_t tSourceLength);