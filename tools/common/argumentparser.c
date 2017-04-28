#include "argumentparser.h"

// TODO: REFACTOR

#include <stdlib.h>
#include <string.h>

#include "legacydefinitions.h"
#include "scriptreader.h"

uint8 BackGroundedAnalyzeArguments(int argc, char *argv[], char CurrentFolderName[], char TextureInputFolder[], char FinalOutputFileDir[], char RomDiskOutputFileDir[], char FileDir[]) {

	int ReadyToRock;
	int DirtyDeedsDoneDirtCheap;

	char CheckText[CheckTextArraySize];
	char CompareWithThis[CheckTextArraySize];

	char SystemCall[FILENAME_MAX * 2];

	if (argc == 1) return(0);

	sprintf(TextureInputFolder, "%s", CurrentFolderName);
	sprintf(FinalOutputFileDir, "%s%cDONE", TextureInputFolder, FOLDER_SLASH);
	sprintf(FileDir, "%s", argv[1]);
	sprintf(RomDiskOutputFileDir, "%s", CurrentFolderName);

	DirtyDeedsDoneDirtCheap = 1;
	while (DirtyDeedsDoneDirtCheap < argc) {

		printf("DirtyDeedsDoneDirtCheap: (%d)\n", DirtyDeedsDoneDirtCheap);

		ReadyToRock = 1;
		sprintf(CheckText, "%s", argv[DirtyDeedsDoneDirtCheap]);

		sprintf(CompareWithThis, "-d");
		if (ReadyToRock && CompareStrings(CompareWithThis, CheckText)) {
			DirtyDeedsDoneDirtCheap++;
			sprintf(TextureInputFolder, "%s", argv[DirtyDeedsDoneDirtCheap]);
			sprintf(FinalOutputFileDir, "%s%cDONE", TextureInputFolder, FOLDER_SLASH);
			ReadyToRock = 0;
		}

		sprintf(CompareWithThis, "-i");
		if (ReadyToRock && CompareStrings(CompareWithThis, CheckText)) {
			DirtyDeedsDoneDirtCheap++;
			sprintf(FileDir, "%s", argv[DirtyDeedsDoneDirtCheap]);

			ReadyToRock = 0;
		}

		sprintf(CompareWithThis, "-o");
		if (ReadyToRock && CompareStrings(CompareWithThis, CheckText)) {
			DirtyDeedsDoneDirtCheap++;
			sprintf(RomDiskOutputFileDir, "%s", argv[DirtyDeedsDoneDirtCheap]);
			ReadyToRock = 0;
		}

		sprintf(CompareWithThis, "-h");
		if (ReadyToRock && CompareStrings(CompareWithThis, CheckText)) return(0);


		DirtyDeedsDoneDirtCheap++;
	}

	sprintf(SystemCall, "mkdir %c%s%c", '"', FinalOutputFileDir, '"');
	system(SystemCall);

	printf("TextureInputFolder: (%s)\n", TextureInputFolder);
	printf("FinalOutputFileDir: (%s)\n", FinalOutputFileDir);
	printf("\n");

	return 1;
}


static int FetchSingleConverterProgramLocation(char ProgramDir[], char OriginalSearchArray[], uint32 OverallEndOffset, FILE* InputFile) {

	uint32 OverallStartOffset;
	uint32 CurrentOffset;

	char CheckText[CheckTextArraySize];
	char SearchForThis[CheckTextArraySize];

	CurrentOffset = 0;
	OverallStartOffset = 0;

	sprintf(SearchForThis, "%s", OriginalSearchArray);
	if (!SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset)) { printf("Couldn't find: (%s)\n\n Aborting!\n", SearchForThis); return(0); }

	sprintf(SearchForThis, "[");
	SearchForString(InputFile, SearchForThis, &CurrentOffset, OverallStartOffset, OverallEndOffset);

	ReadArrayFromInputFile(InputFile, CurrentOffset, CheckText, 0);
	sprintf(ProgramDir, "%s", CheckText);

	return(1);
}

int FetchBackGroundProgramLocations(char CurrentFolderName[], char KallistiImageConverterDir[], char KompressorDir[], char RomDiskDir[]) {

	FILE* InputFile;
	char InputFileDir[FILENAME_MAX];
	char SearchForThis[CheckTextArraySize];

	KallistiImageConverterDir[0] = '\0';
	RomDiskDir[0] = '\0';
	KompressorDir[0] = '\0';

	uint32 InputFileSize;

	sprintf(InputFileDir, "%sTOOLS.cfg", CurrentFolderName);

	InputFile = fopen(InputFileDir, "rb");
	if (InputFile == NULL) {
		return 1;
	}

	fseek(InputFile, 0, SEEK_END);
	InputFileSize = ftell(InputFile);

	fseek(InputFile, 0, SEEK_SET);

	sprintf(SearchForThis, "KMGENC =");
	if (!FetchSingleConverterProgramLocation(KallistiImageConverterDir, SearchForThis, InputFileSize, InputFile)) return(0);

	sprintf(SearchForThis, "GENROMFS =");
	if (!FetchSingleConverterProgramLocation(RomDiskDir, SearchForThis, InputFileSize, InputFile)) return(0);

	sprintf(SearchForThis, "KOMPRESSOR =");
	if (!FetchSingleConverterProgramLocation(KompressorDir, SearchForThis, InputFileSize, InputFile)) return(0);

	fclose(InputFile);

	return(1);
}

void FetchFolderName(char* tSrc, char* tDst) {

	strcpy(tDst, tSrc);
	char* pos = strrchr(tDst, FOLDER_SLASH);

	if (pos == NULL) {
		strcpy(tDst, ".");
	}
	else {
		*(pos + 1) = '\0';
	}
}

void FetchFolderAndOriginalFileName(char OriginalInputFileName[], char InputFolderName[], char RawFileName[]) {

	char InputFileName[FILENAME_MAX];

	int InputFileNameLength;

	int ArrayPosition;
	int OutputArrayPosition;

	int ReadyToRock;


	InputFileNameLength = sprintf(InputFileName, "%s", OriginalInputFileName);

	ArrayPosition = InputFileNameLength;

	ReadyToRock = 1;
	while (ReadyToRock) {
		if (InputFileName[ArrayPosition] == FOLDER_SLASH) ReadyToRock = 0;
		else ArrayPosition--;
	}
	ArrayPosition++;

	sprintf(InputFolderName, "%s", InputFileName);
	InputFolderName[ArrayPosition] = '\0';

	OutputArrayPosition = 0;
	ReadyToRock = 1;
	while (ReadyToRock) {
		if (InputFileName[ArrayPosition + OutputArrayPosition] == '.') ReadyToRock = 0;
		else { RawFileName[OutputArrayPosition] = InputFileName[ArrayPosition + OutputArrayPosition]; OutputArrayPosition++; }
	}
	RawFileName[OutputArrayPosition] = '\0';

}

void getPathWithoutFileExtension(char* tDest, char* tPath) {
	strcpy(tDest, tPath);
	if (!strcmp("", tPath)) return;

	char* pos = strrchr(tDest, '.');
	*pos = '\0';
}