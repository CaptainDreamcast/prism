#include "media.h"

// TODO: REFACTOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/legacydefinitions.h"
#include "../common/scriptreader.h"

void CopyOverTextureFunction(char TemporaryAnimationName[], char FinalOutputFileDir[], char TextureInputFolder[], uint8 FrameID, uint8 FrameAmount, char ConverterDirectory[], char CompressorDirectory[]) {

	char SystemCall[FILENAME_MAX * 2];
	char CopyFunction[FILENAME_MAX];
	int DirtyDeedsDoneDirtCheap;


#ifdef TARGET_OS_MAC

	printf("No MAC support. Maybe later, probably not though.\n");

#elif __linux__

	sprintf(CopyFunction, "cp");

#elif _WIN32

	sprintf(CopyFunction, "copy");

#elif _WIN64

	sprintf(CopyFunction, "copy");

#else 

	printf("The hell did you compile this with, bro?");

#endif


	if (FrameAmount <= 1) {

		sprintf(SystemCall, "%s %c%s%c%s.png%c %c%s%c%d.png%c", CopyFunction, '"', TextureInputFolder, FOLDER_SLASH, TemporaryAnimationName, '"', '"', FinalOutputFileDir, FOLDER_SLASH, FrameID, '"');
		printf("%s\n", SystemCall);
		system(SystemCall);

		printf("---------------- copying %s\n", SystemCall);

		if (!strcmp("", ConverterDirectory)) return;

		sprintf(SystemCall, "%s %c%s%c%d.png%c", ConverterDirectory, '"', FinalOutputFileDir, FOLDER_SLASH, FrameID, '"');
		printf("%s\n", SystemCall);
		system(SystemCall);

		sprintf(SystemCall, "%s%c%d.png", FinalOutputFileDir, FOLDER_SLASH, FrameID);
		printf("%s\n", SystemCall);
		remove(SystemCall);

		sprintf(SystemCall, "%s %c%s%c%d.kmg%c", CompressorDirectory, '"', FinalOutputFileDir, FOLDER_SLASH, FrameID, '"');
		printf("%s\n", SystemCall);
		system(SystemCall);

		sprintf(SystemCall, "%s%c%d.kmg", FinalOutputFileDir, FOLDER_SLASH, FrameID);
		printf("%s\n", SystemCall);
		remove(SystemCall);
	}
	else {

		for (DirtyDeedsDoneDirtCheap = 1; DirtyDeedsDoneDirtCheap <= FrameAmount; DirtyDeedsDoneDirtCheap++) {

			sprintf(SystemCall, "%s %c%s%c%s%d.png%c %c%s%c%d_%d.png%c", CopyFunction, '"', TextureInputFolder, FOLDER_SLASH, TemporaryAnimationName, DirtyDeedsDoneDirtCheap, '"', '"', FinalOutputFileDir, FOLDER_SLASH, FrameID, DirtyDeedsDoneDirtCheap, '"');
			printf("%s\n", SystemCall);
			system(SystemCall);

			if (!strcmp("", ConverterDirectory)) continue;

			sprintf(SystemCall, "%s %c%s%c%d_%d.png%c", ConverterDirectory, '"', FinalOutputFileDir, FOLDER_SLASH, FrameID, DirtyDeedsDoneDirtCheap, '"');
			printf("%s\n", SystemCall);
			system(SystemCall);

			sprintf(SystemCall, "%s%c%d_%d.png", FinalOutputFileDir, FOLDER_SLASH, FrameID, DirtyDeedsDoneDirtCheap);
			printf("%s\n", SystemCall);
			remove(SystemCall);

			sprintf(SystemCall, "%s %c%s%c%d_%d.kmg%c", CompressorDirectory, '"', FinalOutputFileDir, FOLDER_SLASH, FrameID, DirtyDeedsDoneDirtCheap, '"');
			printf("%s\n", SystemCall);
			system(SystemCall);

			sprintf(SystemCall, "%s%c%d_%d.kmg", FinalOutputFileDir, FOLDER_SLASH, FrameID, DirtyDeedsDoneDirtCheap);
			printf("%s\n", SystemCall);
			remove(SystemCall);
		}

	}

}

void CopyOverSoundEffectFunction(char TemporaryAnimationName[], char FinalOutputFileDir[], char TextureInputFolder[], uint8 FrameID) {

	char SystemCall[FILENAME_MAX * 2];
	char CopyFunction[FILENAME_MAX];


#ifdef TARGET_OS_MAC

	printf("No MAC support. Maybe later, probably not though.\n");

#elif __linux__

	sprintf(CopyFunction, "cp");

#elif _WIN32

	sprintf(CopyFunction, "copy");

#elif _WIN64

	sprintf(CopyFunction, "copy");

#else 

	printf("The hell did you compile this with, bro?");

#endif


	sprintf(SystemCall, "%s %c%s/%s.wav%c %c%s/%d.wav%c", CopyFunction, '"', TextureInputFolder, TemporaryAnimationName, '"', '"', FinalOutputFileDir, FrameID, '"');
	printf("%s\n", SystemCall);
	system(SystemCall);
}

uint8 CheckIfCopyingSoundEffectIsNecessary(char TemporaryAnimationName[], uint8* FrameID, uint8* AnimationAmount) {

	FILE* TemporaryNameFile;
	char FileDir[FILENAME_MAX];
	char CheckText[NormalNameSize];
	uint8 CopyOverFightAnimation;
	uint32 BailOutValue;
	uint32 CurrentOffset;

	sprintf(FileDir, "SOUNDEFFECTS.tmp");
	TemporaryNameFile = fopen(FileDir, "rb+");

	fseek(TemporaryNameFile, 0, SEEK_END);
	BailOutValue = ftell(TemporaryNameFile);

	fseek(TemporaryNameFile, 0, SEEK_SET);

	CopyOverFightAnimation = 1;

	CurrentOffset = 0;
	while (CurrentOffset < BailOutValue && CopyOverFightAnimation) {

		fread(CheckText, 1, ActualNameSize, TemporaryNameFile);

		if (CompareStrings(TemporaryAnimationName, CheckText)) {
			*FrameID = (uint8)(CurrentOffset / ActualNameSize);
			CopyOverFightAnimation = 0;
		}

		CurrentOffset += ActualNameSize;
	}

	if (CopyOverFightAnimation) {

		fseek(TemporaryNameFile, 0, SEEK_END);
		fwrite(TemporaryAnimationName, 1, ActualNameSize, TemporaryNameFile);
		*FrameID = *AnimationAmount;
		(*AnimationAmount)++;

	}

	fclose(TemporaryNameFile);

	return(CopyOverFightAnimation);
}
