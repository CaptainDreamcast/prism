#include "../include/file.h"

#include <stdio.h>
#include <windows.h>

#include "../include/log.h"
#include "../include/memoryhandler.h"
#include "../include/system.h"


static struct {
	char cwd[1024];
} gData;

void initFileSystem(){
	log("Initiate file system.");
	sprintf(gData.cwd, "/");
	debugString(gData.cwd);
}

void setFileSystem(char* path){
	(void)path;
}

const char* getFileSystem() {
	return NULL;
}


void setWorkingDirectory(char* path) {

	sprintf(gData.cwd, "%s", path);
	debugString(gData.cwd);

	int l = strlen(gData.cwd);
	if(gData.cwd[l-1] != '/') {
		gData.cwd[l] = '/';
		gData.cwd[l+1] = '\0';
	}
}

const char* getWorkingDirectory() {
	return gData.cwd;
}

void getFullPath(char* tDest, char* tPath) {
	if (tPath[0] == '$') tPath+=4;

	if(tPath[0] == '/') sprintf(tDest, ".%s", tPath);
	else sprintf(tDest, ".%s%s", gData.cwd, tPath);
}

FileHandler fileOpen(char* tPath, int tFlags){
	char path[1024];
	getFullPath(path, tPath);

	debugLog("Open file.");
	debugString(tPath);
	debugString(gData.fileSystem);
	debugString(gData.cwd);

	char flags[100];
	if (tFlags == O_RDONLY) {
		sprintf(flags, "r");
	}
	else {
		logError("Unrecognized read mode");
		logErrorInteger(tFlags)
		abortSystem();
	}

	return fopen(path, flags);
}

int fileClose(FileHandler tHandler) {
	return fclose(tHandler);
}
size_t fileRead(FileHandler tHandler, void* tBuffer, size_t tCount) {
	return fread(tBuffer, 1, tCount, tHandler);
}
size_t fileWrite(FileHandler tHandler, const void* tBuffer, size_t tCount) {
	return fwrite(tBuffer, 1, tCount, tHandler);
}
size_t fileSeek(FileHandler tHandler, size_t tOffset, int tWhence)  {
	return fseek(tHandler, tOffset, tWhence);
}
size_t fileTell(FileHandler tHandler) {
	return ftell(tHandler);
}
size_t fileTotal(FileHandler tHandler){
	fseek(tHandler, 0L, SEEK_END);
	size_t size = ftell(tHandler);
	rewind(tHandler);

	return size;
}
int fileUnlink(char* tPath) {
	return remove(tPath);
}

void* fileMemoryMap(FileHandler tHandler) {
	return NULL;
}

void mountRomdisk(char* tFilePath, char* tMountPath) {
	logError("Mounting romdisks not implemented under Windows.");
	abortSystem();
}

void unmountRomdisk(char* tMountPath) {
	logError("Mounting romdisks not implemented under Windows.");
	abortSystem();
}

void printDirectory(char* tPath) {
	char path[1024];
	getFullPath(path, tPath);

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	hFind = FindFirstFile(path, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND) {
			logg("No files in directory.");
			logString(path);
			return;
		}
		else {
			logError("Unable to read directory");
			logErrorString(path);
			abortSystem();
		}
	}

	do {
		logString(findFileData.cFileName);
	} while (FindNextFile(hFind, &findFileData));

	FindClose(hFind);
}

