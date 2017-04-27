#include "tari/file.h"

#include <stdio.h>
#include <windows.h>

#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"
#include "tari/datastructures.h"

typedef struct {
	char mMount[1024];
	char mPath[1024];
} RomdiskMapping;

static struct {
	char cwd[1024];
	char mFileSystem[1024];
	StringMap mRomdiskMappings;
} gData;


void initFileSystem(){
	logg("Initiate file system.");
	sprintf(gData.cwd, "/");
	gData.mFileSystem[0] = '\0';
	debugString(gData.cwd);
	gData.mRomdiskMappings = new_string_map();
}

static void expandPath(char* tDest, char* tPath) {
	strcpy(tDest, tPath);
	if (tDest[0] != '/') return;

	char potentialMount[1024];
	strcpy(potentialMount, tDest + 1);
	char* endPos = strchr(potentialMount, '/');
	if (endPos != NULL) *endPos = '\0';

	if (!strcmp("rd", potentialMount) || !strcmp("pc", potentialMount)) {
		if (endPos == NULL) strcpy(tDest, "/");
		else sprintf(tDest, "/%s", endPos + 1);
		return;
	}

	int isMount = string_map_contains(&gData.mRomdiskMappings, potentialMount);
	if (!isMount) return;

	RomdiskMapping* e = string_map_get(&gData.mRomdiskMappings, potentialMount);
	
	sprintf(tDest, "%s%s", e->mPath, endPos + 1);
}

void setFileSystem(char* path){
	char expandedPath[1024];
	expandPath(expandedPath, path);
	strcpy(gData.mFileSystem, expandedPath);
}

const char* getFileSystem() {
	return gData.mFileSystem;
}


void setWorkingDirectory(char* path) {
	char expandedPath[1024];
	expandPath(expandedPath, path);
	strcpy(gData.cwd, expandedPath);
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

	if (tPath[0] == '/') {
		char expandedPath[1024];
		expandPath(expandedPath, tPath);
		sprintf(tDest, ".%s", expandedPath);
	}
	
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
		sprintf(flags, "rb");
	} else if (tFlags == O_WRONLY) {
		sprintf(flags, "wb+");
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
	(void)tHandler;
	return NULL;
}


void mountRomdisk(char* tFilePath, char* tMountPath) {
	int isAlreadyMounted = string_map_contains(&gData.mRomdiskMappings, tMountPath);
	if (isAlreadyMounted) {
		logError("Unable to mount. Already mounted.");
		logErrorString(tMountPath);
		abortSystem();
	}

	char path[1024], folderPath[104];
	getPathWithoutFileExtension(path, tFilePath);
	if(tFilePath[0] == '/') sprintf(folderPath, "%s/", path);
	else sprintf(folderPath, "/%s/", path);

	RomdiskMapping* e = allocMemory(sizeof(RomdiskMapping));
	strcpy(e->mMount, tMountPath);
	strcpy(e->mPath, folderPath);
	string_map_push_owned(&gData.mRomdiskMappings, tMountPath, e);
}

void unmountRomdisk(char* tMountPath) {
	string_map_remove(&gData.mRomdiskMappings, tMountPath);
}

void printDirectory(char* tPath) {
	char path[1024];
	wchar_t wpath[1024];
	getFullPath(path, tPath);

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	mbstowcs(wpath, path, 1024);

	hFind = FindFirstFile(wpath, &findFileData);
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
		logWString(findFileData.cFileName);
	} while (FindNextFile(hFind, &findFileData));

	FindClose(hFind);
}

