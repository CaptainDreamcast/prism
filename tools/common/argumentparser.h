#pragma once

// TODO: REFACTOR

#include "int.h"

uint8 BackGroundedAnalyzeArguments(int argc, char *argv[], char CurrentFolderName[], char TextureInputFolder[], char FinalOutputFileDir[], char RomDiskOutputFileDir[], char FileDir[]);
int FetchBackGroundProgramLocations(char CurrentFolderName[], char KallistiImageConverterDir[], char KompressorDir[], char RomDiskDir[]);

void FetchFolderName(char OriginalInputFileName[], char InputFolderName[]);
void FetchFolderAndOriginalFileName(char OriginalInputFileName[], char InputFolderName[], char RawFileName[]);
void getPathWithoutFileExtension(char* tDest, char* tPath);