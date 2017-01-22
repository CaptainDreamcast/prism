#include "include/file.h"

#include "include/log.h"

char* getPureFileName(char* path){
	char* pos = strrchr(path,'/');

	if(pos == NULL) return path;
	else return pos+1;
}

Buffer fileToBuffer(char* tFileDir){
	Buffer ret;

	size_t bufferLength;
	file_t file;
	char* mipMapData;

	file = fileOpen(tFileDir, O_RDONLY);

	if (file == FILEHND_INVALID) {
		logError("Couldn't open file: Try returning to menu...");
		logErrorString(tFileDir);
		arch_menu();
	}

	bufferLength = fileTotal(file);
	debugInteger(bufferLength);

	mipMapData = fileMemoryMap(file);

	ret.isOwned = 0;
	ret.data = mipMapData;
	ret.length = bufferLength;

	fileClose(file);

	return ret;
}

void freeBuffer(Buffer buffer){
	if(buffer.isOwned){
		debugLog("Freeing owned memory");
		debugInteger(buffer.length);
		free(buffer.data);
	}
	buffer.data = NULL;
	buffer.length = 0;
	buffer.isOwned = 0;
}
