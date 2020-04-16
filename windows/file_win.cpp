#include "prism/file.h"

#include <stdio.h>
#include <filesystem>

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/debug.h"
#include "prism/datastructures.h"
#include "prism/windows/romdisk_win.h"

static struct {
	char cwd[1024];
	char mFileSystem[1024];
	int mIsUsingRomdisk;
} gPrismWindowsFileData;


extern char romdisk_buffer[];
extern int romdisk_buffer_length;

void initFileSystem(){
	setActiveFileSystemOnStartup();
	sprintf(gPrismWindowsFileData.cwd, "/");
	sprintf(gPrismWindowsFileData.mFileSystem, ".");
	debugString(gPrismWindowsFileData.cwd);

	initRomdisks();

	if (romdisk_buffer_length) {
		mountRomdiskFromBuffer(makeBuffer(romdisk_buffer, romdisk_buffer_length), "ASSETS");
		strcpy(gPrismWindowsFileData.mFileSystem, "/ASSETS");
	}
	else if (isFile("assets.pak")) {
		mountRomdisk("assets.pak", "ASSETS");
		strcpy(gPrismWindowsFileData.mFileSystem, "/ASSETS");
	}	
}

void shutdownFileSystem() {
	shutdownRomdisks();
}

static void expandPath(char* tDest, const char* tPath) {
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
}

void setFileSystem(const char* path){
	(void)path;
}

const char* getFileSystem() {
	return gPrismWindowsFileData.mFileSystem;
}


void setWorkingDirectory(const char* path) {
	char expandedPath[1024], absolutePath[1024];
	if (path[0] != '/') {
		sprintf(absolutePath, "%s%s", gPrismWindowsFileData.cwd, path);
	}
	else {
		strcpy(absolutePath, path);
	}

	expandPath(expandedPath, absolutePath);
	strcpy(gPrismWindowsFileData.cwd, expandedPath);
	debugString(gPrismWindowsFileData.cwd);

	int l = strlen(gPrismWindowsFileData.cwd);
	if(gPrismWindowsFileData.cwd[l-1] != '/') {
		gPrismWindowsFileData.cwd[l] = '/';
		gPrismWindowsFileData.cwd[l+1] = '\0';
	}
}

const char* getWorkingDirectory() {
	return gPrismWindowsFileData.cwd;
}

static int isAbsoluteWindowsDirectory(const char* tPath) {
	return tPath[1] == ':';
}

void getFullPath(char* tDest, const char* tPath) {
	if (isRomdiskPath(tPath) || isAbsoluteWindowsDirectory(tPath)) {
		if (tPath[0] == '$') tPath++;
		strcpy(tDest, tPath);
		return;
	}

	if (tPath[0] == '$') tPath+=4;

	if (tPath[0] == '/') {
		char expandedPath[1024];
		expandPath(expandedPath, tPath);
		sprintf(tDest, "%s%s", gPrismWindowsFileData.mFileSystem, expandedPath);
	}
	else sprintf(tDest, "%s%s%s", gPrismWindowsFileData.mFileSystem, gPrismWindowsFileData.cwd, tPath);
}

FileHandler fileOpen(const char* tPath, int tFlags){

	char path[1024];
	getFullPath(path, tPath);

	if (isRomdiskPath(path)) {
		return fileOpenRomdisk(path, tFlags);
	}

	debugLog("Open file.");
	debugString(tPath);
	debugString(gPrismWindowsFileData.mFileSystem);
	debugString(gPrismWindowsFileData.cwd);

	char flags[100];
	flags[0] = '\0';
	if (tFlags == O_RDONLY) {
		sprintf(flags, "rb");
	} else if (tFlags == O_WRONLY) {
		sprintf(flags, "wb+");
	}
	else {
		logError("Unrecognized read mode");
		logErrorInteger(tFlags)
			recoverFromError();
	}

	if (isInDevelopMode() && strchr(path, '-')) {
		logErrorFormat("Illegal character '-' in path %s. Aborting.", path);
		recoverFromError();
	}

	return fopen(path, flags);
}

int fileClose(FileHandler tHandler) {
	if (isRomdiskFileHandler(tHandler)) return fileCloseRomdisk(tHandler);

	return fclose(tHandler);
}
size_t fileRead(FileHandler tHandler, void* tBuffer, size_t tCount) {
	if (isRomdiskFileHandler(tHandler)) return fileReadRomdisk(tHandler, tBuffer, tCount);

	return fread(tBuffer, 1, tCount, tHandler);
}
size_t fileWrite(FileHandler tHandler, const void* tBuffer, size_t tCount) {
	if (isRomdiskFileHandler(tHandler)) {
		logError("Unable to write to romdisk file.");
		logErrorInteger(tHandler);
		recoverFromError();
	}

	return fwrite(tBuffer, 1, tCount, tHandler);
}
size_t fileSeek(FileHandler tHandler, size_t tOffset, int tWhence)  {
	if (isRomdiskFileHandler(tHandler)) return fileSeekRomdisk(tHandler, tOffset, tWhence);

	return fseek(tHandler, tOffset, tWhence);
}
size_t fileTell(FileHandler tHandler) {
	if (isRomdiskFileHandler(tHandler)) return fileTellRomdisk(tHandler);

	return ftell(tHandler);
}
size_t fileTotal(FileHandler tHandler){
	if (isRomdiskFileHandler(tHandler)) return fileTotalRomdisk(tHandler);

	fseek(tHandler, 0L, SEEK_END);
	size_t size = ftell(tHandler);
	rewind(tHandler);

	return size;
}

void fileFlush(FileHandler tHandler) {
	if (isRomdiskFileHandler(tHandler)) return;
	fflush(tHandler);
}

int fileUnlink(const char* tPath) {
	return remove(tPath);
}

void* fileMemoryMap(FileHandler tHandler) {
	(void)tHandler;
	return NULL;
}

void createDirectory(const char * tPath)
{
#ifndef __EMSCRIPTEN__
	if (!isDirectory(tPath)) {
		char path[1024];
		getFullPath(path, tPath);
		std::experimental::filesystem::create_directories(path);
	}
#endif
}

void mountRomdiskFromBuffer(Buffer b, const char * tMountPath)
{
	mountRomdiskWindowsFromBuffer(b, tMountPath);
}

void mountRomdisk(const char* tFilePath, const char* tMountPath) {
	
	char fullPath[1024];
	expandPath(fullPath, tFilePath);
	mountRomdiskWindows(fullPath, tMountPath);
}

void unmountRomdisk(const char* tMountPath) {
	unmountRomdiskWindows(tMountPath);
}

#ifdef _WIN32

#include <Windows.h>

#endif

void printDirectory(const char* tPath) {
#ifdef _WIN32
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
			recoverFromError();
		}
	}

	do {
		logWString(findFileData.cFileName);
	} while (FindNextFile(hFind, &findFileData));

	FindClose(hFind);
#endif
}