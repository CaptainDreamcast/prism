#include "prism/file.h"

#include <dirent.h> 
#include <kos.h>

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"


static struct {
	char cwd[1024];
	char fileSystem[1024];
} gData;

void initFileSystem(){
	logg("Initiate file system.");
	sprintf(gData.cwd, "/");
	debugString(gData.cwd);
}

void setFileSystem(char* path){
	logg("Setting file system.");
	if(path[0] != '/'){
		logError("Invalid filesystem!");	
		logErrorString(path);
	}

	sprintf(gData.fileSystem, "%s", path);
	debugString(gData.fileSystem);
	
}

const char* getFileSystem() {
	return gData.fileSystem;
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

void getFullPath(char* tDest, const char* tPath) {
	if(tPath[0] == '$') sprintf(tDest, "%s", tPath+1);
	else if(tPath[0] == '/') sprintf(tDest, "%s%s", gData.fileSystem, tPath);
	else sprintf(tDest, "%s%s%s", gData.fileSystem, gData.cwd, tPath);
}

FileHandler fileOpen(const char* tPath, int tFlags){
	char path[1024];
	getFullPath(path, tPath);

	debugLog("Open file.");
	debugString(tPath);
	debugString(gData.fileSystem);
	debugString(gData.cwd);

	return fs_open(path, tFlags);
}

int fileClose(FileHandler tHandler) {
	return fs_close(tHandler);
}
size_t fileRead(FileHandler tHandler, void* tBuffer, size_t tCount) {
	return fs_read(tHandler, tBuffer, tCount);
}
size_t fileWrite(FileHandler tHandler, const void* tBuffer, size_t tCount) {
	return fs_write(tHandler, tBuffer, tCount);
}
size_t fileSeek(FileHandler tHandler, size_t tOffset, int tWhence)  {
	return fs_seek(tHandler, tOffset, tWhence);
}
size_t fileTell(FileHandler tHandler) {
	return fs_tell(tHandler);
}
size_t fileTotal(FileHandler tHandler){
	return fs_total(tHandler);
}
int fileUnlink(char* tPath) {
	return fs_unlink(tPath);
}
void* fileMemoryMap(FileHandler tHandler) {
	return fs_mmap(tHandler);
}

void fixMountPath(char* tDst, char* tSrc) {
	if(tSrc[0] != '/') {
		sprintf(tDst, "/%s", tSrc);	
	} else {
		strcpy(tDst, tSrc);
	}
}


void mountRomdisk(char* tFilePath, char* tMountPath) {
	char mountPath[200];
	fixMountPath(mountPath, tMountPath);

	file_t romDiskFile;
	uint8* romDiskBuffer;
	long romDiskSize;

	romDiskFile = fileOpen(tFilePath, O_RDONLY);
	romDiskSize = fileTotal(romDiskFile);

	romDiskBuffer = malloc(romDiskSize);
	fileRead(romDiskFile, romDiskBuffer, romDiskSize);
	fs_romdisk_mount(mountPath, romDiskBuffer, 1);

	fileClose(romDiskFile);
}

void unmountRomdisk(char* tMountPath) {
	char mountPath[200];
	fixMountPath(mountPath, tMountPath);

	fs_romdisk_unmount(mountPath);
}

void printDirectory(char* tPath) {
	char path[1024];
	getFullPath(path, tPath);

	DIR* d;	
	d = opendir(path);
	if (d)
	{
		struct dirent *dir;
		while ((dir = readdir(d)) != NULL)
		{
			logString(dir->d_name);
		}

		closedir(d);
	}
}

