#include "finalize.h"

// TODO: REFACTOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/legacydefinitions.h"

void FinalizeStoryBoard(uint8 WhichStoryBoard, char RomDiskOutputFileDir[], char RomDiskDirectory[], char FinalOutputFileDir[]) {

	char SystemCall[FILENAME_MAX * 3];

	sprintf(SystemCall, "NAMES.tmp");
	printf("%s\n", SystemCall);
	remove(SystemCall);

	sprintf(SystemCall, "SOUNDEFFECTS.tmp");
	printf("%s\n", SystemCall);
	remove(SystemCall);

	if (!strcmp("", RomDiskDirectory)) {
		sprintf(SystemCall, "xcopy /E /I /Y \"%s\" \"%s%c%d\"", FinalOutputFileDir, RomDiskOutputFileDir, FOLDER_SLASH, WhichStoryBoard);
		printf("%s\n", SystemCall);
		system(SystemCall);
	}
	else {
		sprintf(SystemCall, "%s -d %c%s%c -f %c%s/%d.sbd%c", RomDiskDirectory, '"', FinalOutputFileDir, '"', '"', RomDiskOutputFileDir, WhichStoryBoard, '"');
		printf("%s\n", SystemCall);
		system(SystemCall);
	}
}