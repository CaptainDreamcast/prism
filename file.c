#include "include/file.h"

#include "include/log.h"
#include "include/memoryhandler.h"
#include "include/system.h"

char* getPureFileName(char* path) {
	debugLog("Getting pure filename.");
	char* pos = strrchr(path, '/');

	if (pos == NULL) return path;
	else return pos + 1;
}


char* getFileExtension(char* tPath) {
	char* pos = strrchr(tPath, '.');

	if (pos == NULL) {
		logError("Unable to find file ending.");
		logErrorString(tPath);
		abortSystem();
	}

	return pos + 1;
}

void getPathWithoutFileExtension(char* tDest, char* tPath) {
	strcpy(tDest, tPath);

	char* pos = strrchr(tDest, '.');
	*pos = '\0';
}

void getPathWithNumberAffixedFromAssetPath(char* tDest, const char* tSrc, int i) {
	char name[1024];
	strcpy(name, tSrc);

	char* pos = strrchr(name, '.');
	if (pos == NULL) {
		sprintf(tDest, "%s%d", tSrc, i);
	}
	else {
		pos[0] = '\0';
		sprintf(tDest, "%s%d.%s", name, i, pos + 1);
		pos[0] = '.';
	}

}

static int isFileMemoryMapped(FileHandler tFile) {
	void* data = fileMemoryMap(tFile);
	return data != NULL;
}

Buffer fileToBuffer(char* tFileDir) {
	debugLog("Reading file to Buffer.");
	Buffer ret;

	size_t bufferLength;
	FileHandler file;
	char* data;

	file = fileOpen(tFileDir, O_RDONLY);

	if (file == FILEHND_INVALID) {
		logError("Couldn't open file.");
		logErrorString(tFileDir);
		logErrorString(getWorkingDirectory());
		logErrorString(getFileSystem());
		abortSystem();
	}

	bufferLength = fileTotal(file);
	debugInteger(bufferLength);

	if (isFileMemoryMapped(file)) {
		data = fileMemoryMap(file);
		ret.mIsOwned = 0;
	}
	else {
		data = allocMemory(bufferLength);
		fileRead(file, data, bufferLength);
		ret.mIsOwned = 1;
	}

	ret.mData = data;
	ret.mLength = bufferLength;

	fileClose(file);

	return ret;
}

void freeBuffer(Buffer buffer) {
	debugLog("Freeing buffer.");
	if (buffer.mIsOwned) {
		debugLog("Freeing owned memory");
		debugInteger(buffer.mLength);
		freeMemory(buffer.mData);
	}
	buffer.mData = NULL;
	buffer.mLength = 0;
	buffer.mIsOwned = 0;
}

void appendTerminationSymbolToBuffer(Buffer* tBuffer) {
	if (!tBuffer->mIsOwned) {
		char* nData = allocMemory(tBuffer->mLength + 1);
		memcpy(nData, tBuffer->mData, tBuffer->mLength);
		tBuffer->mData = nData;
		tBuffer->mIsOwned = 1;
	}
	else {
		tBuffer->mData = reallocMemory(tBuffer->mData, tBuffer->mLength + 1);
	}


	char* buf = tBuffer->mData;
	buf[tBuffer->mLength] = '\0';
	tBuffer->mLength++;
}
