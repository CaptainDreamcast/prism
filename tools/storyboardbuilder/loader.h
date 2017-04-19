#pragma once

// TODO: REFACTOR

#include <stdio.h>

#include "../common/int.h"

long LoadStoryBoardData(FILE* InputFile, uint32 OverallStartOffset, char TextureInputFolder[], char FinalOutputFileDir[], char RomDiskOutputFileDir[], char ConverterDirectory[], char CompressorDirectory[], char CharacterName[], uint8* WhichStoryBoard);