// TODO: REFACTOR

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../common/int.h"
#include "../common/legacydefinitions.h"
#include "../common/argumentparser.h"

#include "loader.h"
#include "finalize.h"

int main(int argc, char *argv[]) {

	FILE* InputFile;

	uint8 WhichStoryBoard;
	uint32 CurrentOffset;

	char CharacterName[NormalNameSize];
	char TextureInputFolder[FILENAME_MAX];
	char FinalOutputFileDir[FILENAME_MAX];
	char RomDiskOutputFileDir[FILENAME_MAX];
	char ConverterDirectory[FILENAME_MAX];
	char CompressorDirectory[FILENAME_MAX];
	char RomDiskDirectory[FILENAME_MAX];
	char CurrentFolderName[FILENAME_MAX];
	char InputFileDir[FILENAME_MAX];

	FetchFolderName(argv[0], CurrentFolderName);
	if (!FetchBackGroundProgramLocations(CurrentFolderName, ConverterDirectory, CompressorDirectory, RomDiskDirectory)) return(0);

	if (!BackGroundedAnalyzeArguments(argc, argv, CurrentFolderName, TextureInputFolder, FinalOutputFileDir, RomDiskOutputFileDir, InputFileDir)) {

		printf("\n");
		printf("Dolmexica Engine Compiler (Dolmexiler) for Story Boards\n");
		printf("Version 1.0.0 - Source available at dolmexicaengine.sourceforge.net\n");
		printf("\n");
		printf("Usage:\n");
		printf("\n");
		printf("-h: Show this help.\n");
		printf("-i: The location of the input text file. Must be using raw ASCII, so use editors compatible with that.\n");
		printf("-d: The input directory where the raw images and Sound Effects of your Story Board are located.\n");
		printf("-o: The output directory where your D-Engine files are created.\n");
		printf("\n");
		printf("\n");
		printf("Copyright 2012 - Josh Tari - GNU General Public License v3\n");
		printf("\n");

		return(1);
	}

	InputFile = fopen(InputFileDir, "rb");
	fseek(InputFile, 0, SEEK_SET);

	CurrentOffset = 0;


	CurrentOffset = LoadStoryBoardData(InputFile, CurrentOffset, TextureInputFolder, FinalOutputFileDir, RomDiskOutputFileDir, ConverterDirectory, CompressorDirectory, CharacterName, &WhichStoryBoard);
	if (CurrentOffset == -1) {
		fclose(InputFile);
		return(0);
	}

	FinalizeStoryBoard(WhichStoryBoard, RomDiskOutputFileDir, RomDiskDirectory, FinalOutputFileDir);

	fclose(InputFile);

	return(0);
}