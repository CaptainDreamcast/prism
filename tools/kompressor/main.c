#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../include/tari/quicklz.h"

#include "../common/argumentparser.h"

typedef struct {
	int mLength;
	void* mData;

} Buffer;

static Buffer fileToBufferTool(char* tFileDir) {
	Buffer ret;

	size_t bufferLength;
	FILE* file;
	char* data;

	file = fopen(tFileDir, "rb");

	if (file == NULL) {
		printf("Couldn't open file.\n");
		printf("tFileDir: %s\n", tFileDir);
		exit(1);
	}

	fseek(file, 0L, SEEK_END);
	bufferLength = ftell(file);
	rewind(file);

	data = malloc(bufferLength);
	fread(data, 1, bufferLength, file);
	ret.mData = data;
	ret.mLength = bufferLength;

	fclose(file);

	return ret;
}

static void bufferToFileTool(char* tFileDir, Buffer tBuffer) {
	FILE* file = fopen(tFileDir, "wb+");
	fwrite(tBuffer.mData, 1, tBuffer.mLength, file);
	fclose(file);
}


static void compressBufferTool(Buffer* tBuffer) {
	Buffer src = *tBuffer;
	Buffer dst = *tBuffer;

	qlz_state_compress state_compress;

	dst.mData = malloc(src.mLength + 400);
	dst.mLength = qlz_compress(src.mData, dst.mData, src.mLength, &state_compress);
	dst.mData = realloc(dst.mData, dst.mLength);

	free(src.mData);

	*tBuffer = dst;
}

static void parseArgument(char** argv, int i, char* tKMGFileName, char* tPKGFileName) {

	strcpy(tKMGFileName, argv[i]);

	char fileNameWithoutExtension[1024];
	getPathWithoutFileExtension(fileNameWithoutExtension, tKMGFileName);

	sprintf(tPKGFileName, "%s.pkg", fileNameWithoutExtension);
}

int main(int argc, char *argv[]) {
	
	if (argc == 1) {

		printf("\n");
		printf("Dolmexica Engine Kompressor (Image to PKG Converter)\n");
		printf("Version 1.0.1 - Source available at https://github.com/CaptainDreamcast/libtari \n");
		printf("\n");
		printf("Usage:\n");
		printf("\n");
		printf("This is a drag and drop tool. Select all the files you wish to compress and move them on the executable. Supports batch processing and creates the output files in the folder the input files are located at.\n");
		printf("\n");
		printf("\n");
		printf("This is an application which just takes QuickLZ and turns it into a usable drag-'n-drop tool. There is nothing for me to take credit for, all of that goes to the QuickLZ dev team. Check out their website at 'http://www.quicklz.com/'. This tool complies with their rule that tools using QLZ must be released under the GNU General Public License (or purchase a license).\n");
		printf("\n");
		printf("License: GNU General Public License v3\n");
		printf("\n");
		return 1;
	}



	int i;
	for (i = 1; i < argc; i++) {
		char fileNameKMG[1024];
		char fileNamePKG[1024];

		parseArgument(argv, i, fileNameKMG, fileNamePKG);

		Buffer b = fileToBufferTool(fileNameKMG);
		compressBufferTool(&b);
		bufferToFileTool(fileNamePKG, b);
		free(b.mData);
	}

	return 0;
}