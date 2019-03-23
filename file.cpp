#include "prism/file.h"

#include <sys/stat.h>

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"

using namespace std;

char* getPureFileName(char* path) {
	debugLog("Getting pure filename.");
	char* pos = strrchr(path, '/');

	if (pos == NULL) return path;
	else return pos + 1;
}


const char* getFileExtension(const char* tPath) {
	const char* pos = strrchr(tPath, '.');

	if (pos == NULL) {
		logError("Unable to find file ending.");
		logErrorString(tPath);
		recoverFromError();
	}

	return pos + 1;
}

char* getFileExtension(char* tPath) {
	char* pos = strrchr(tPath, '.');

	if (pos == NULL) {
		logError("Unable to find file ending.");
		logErrorString(tPath);
		recoverFromError();
	}

	return pos + 1;
}

void getPathWithoutFileExtension(char* tDest, char* tPath) {
	strcpy(tDest, tPath);
	if (!strcmp("", tPath)) return;

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

void getPathToFile(char * tDest, char * tPath)
{
	strcpy(tDest, tPath);
	char* pathEnd = strrchr(tDest, '/');
	if (pathEnd != NULL) pathEnd[1] = '\0';
}

static int isFileMemoryMapped(FileHandler tFile) {
	void* data = fileMemoryMap(tFile);
	return data != NULL;
}

int isFile(char* tPath) {
	if(isDirectory(tPath)) return 0;

	FileHandler file = fileOpen(tPath, O_RDONLY);
	if (file == FILEHND_INVALID) return 0;
	fileClose(file);
	return 1;
}

int isDirectory(char* tPath) {
	struct stat sb;
	char path[1024];
	getFullPath(path, tPath);

	return (stat(path, &sb) == 0 && (sb.st_mode & S_IFDIR));
}


static Buffer makeBufferInternal(void * tData, uint32_t tLength, int tIsOwned)
{
	Buffer b;
	b.mData = tData;
	b.mLength = tLength;
	b.mIsOwned = tIsOwned;
	return b;
}

Buffer makeBuffer(void * tData, uint32_t tLength)
{
	return makeBufferInternal(tData, tLength, 0);
}

Buffer makeBufferOwned(void * tData, uint32_t tLength)
{
	return makeBufferInternal(tData, tLength, 1);
}

Buffer makeBufferEmptyOwned() {
	return makeBufferInternal(NULL, 0, 1);
}

Buffer copyBuffer(Buffer tBuffer) {
	Buffer ret;
	ret.mIsOwned = 1;
	ret.mLength = tBuffer.mLength;
	ret.mData = allocMemory(ret.mLength + 2);
	memcpy(ret.mData, tBuffer.mData, ret.mLength);

	return ret;
}

Buffer fileToBuffer(const char* tFileDir) {
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
		recoverFromError();
	}

	bufferLength = fileTotal(file);
	debugInteger(bufferLength);

	if (isFileMemoryMapped(file)) {
		data = (char*)fileMemoryMap(file);
		ret.mIsOwned = 0;
	}
	else {
		data = (char*)allocMemory(bufferLength);
		fileRead(file, data, bufferLength);
		ret.mIsOwned = 1;
	}

	ret.mData = data;
	ret.mLength = bufferLength;

	fileClose(file);

	return ret;
}

void bufferToFile(char* tFileDir, Buffer tBuffer) {
	FileHandler file = fileOpen(tFileDir, O_WRONLY);
	fileWrite(file, tBuffer.mData, tBuffer.mLength);
	fileClose(file);
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
		char* nData = (char*)allocMemory(tBuffer->mLength + 1);
		memcpy(nData, tBuffer->mData, tBuffer->mLength);
		tBuffer->mData = nData;
		tBuffer->mIsOwned = 1;
	}
	else {
		tBuffer->mData = reallocMemory(tBuffer->mData, tBuffer->mLength + 1);
	}


	char* buf = (char*)tBuffer->mData;
	buf[tBuffer->mLength] = '\0';
	tBuffer->mLength++;
}

void fileToMemory(void* tDst, int tSize, char* tPath) {
	Buffer b = fileToBuffer(tPath);
	if (b.mLength != (unsigned int)tSize) {
		logError("File and memory struct have different sizes!");
		logErrorString(tPath);
		logErrorInteger(tSize);
		logErrorInteger(b.mLength);
		recoverFromError();
	}
	memcpy(tDst, b.mData, tSize);

	freeBuffer(b);
}

BufferPointer getBufferPointer(Buffer tBuffer)
{
	return (BufferPointer)tBuffer.mData;
}

void readFromBufferPointer(void * tDst, BufferPointer* tPointer, uint32_t tSize)
{
	memcpy(tDst, *tPointer, tSize);
	(*tPointer) += tSize;
}

int readIntegerFromTextStreamBufferPointer(BufferPointer* tPointer) {
	int value;
	int size;
	int items = sscanf(*tPointer, "%d%n", &value, &size);
	if (items != 1) {
		logWarning("Unable to read integer value from stream.");
		value = 0;
	}
	(*tPointer) += size;
	return value;
}

double readFloatFromTextStreamBufferPointer(BufferPointer * tPointer)
{
	double value;
	int size;
	int items = sscanf(*tPointer, "%lf%n", &value, &size);
	if (items != 1) {
		logWarning("Unable to read float value from stream.");
		value = 0;
	}
	(*tPointer) += size;
	return value;
}

std::string readStringFromTextStreamBufferPointer(BufferPointer * tPointer)
{
	char value[1000];
	int size;
	int items = sscanf(*tPointer, "%s%n", value, &size);
	if (items != 1) {
		logWarning("Unable to read float value from stream.");
		value[0] = '\0';
	}
	(*tPointer) += size;
	return std::string(value);
}

string readLineFromTextStreamBufferPointer(BufferPointer * tPointer)
{
	string s;
	while (**tPointer != '\n') {
		if(**tPointer != '\r') s += **tPointer;
		(*tPointer)++;
	}
	(*tPointer)++;
	return s;
}

void appendBufferChar(Buffer* tBuffer, char tChar) {
	appendBufferString(tBuffer, &tChar, 1);
}

void appendBufferUint32(Buffer* tBuffer, uint32_t tInteger) {
	appendBufferString(tBuffer, (char*)(&tInteger), sizeof(uint32_t));
}

void appendBufferInt32(Buffer* tBuffer, int32_t tInteger) {
	appendBufferString(tBuffer, (char*)(&tInteger), sizeof(int32_t));
}

void appendBufferInteger(Buffer* tBuffer, int tInteger) {
	appendBufferString(tBuffer, (char*)(&tInteger), sizeof(int));

}

void appendBufferFloat(Buffer* tBuffer, float tFloat) {
	appendBufferString(tBuffer, (char*)(&tFloat), sizeof(float));
}

void appendBufferString(Buffer* tBuffer, char* tString, int tLength) {
	tBuffer->mLength += tLength;
	tBuffer->mData = reallocMemory(tBuffer->mData, tBuffer->mLength + 2);

	char* pos = &(((char*)tBuffer->mData)[tBuffer->mLength - tLength]);
	memcpy(pos, tString, tLength);
}

void appendBufferBuffer(Buffer* tBuffer, Buffer tInputBuffer) {
	appendBufferString(tBuffer, (char*)tInputBuffer.mData, tInputBuffer.mLength);
}
