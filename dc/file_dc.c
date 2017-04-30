#include "tari/file.h"

#include <dirent.h> 
#include <kos.h>

#include "tari/log.h"
#include "tari/memoryhandler.h"
#include "tari/system.h"


static struct {
	char cwd[1024];
	char fileSystem[1024];
} gData;

void initFileSystem(){
	logg("Initiate file system.");
	sprintf(gData.cwd, "/");
	gData.fileSystem[0] = '\0';
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

void getFullPath(char* tDest, char* tPath) {
	if(tPath[0] == '$') sprintf(tDest, "%s", tPath+1);
	else if(tPath[0] == '/') sprintf(tDest, "%s%s", gData.fileSystem, tPath);
	else sprintf(tDest, "%s%s%s", gData.fileSystem, gData.cwd, tPath);
}

FileHandler fileOpen(char* tPath, int tFlags){
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






void mountRomdisk(char* tFilePath, char* tMountPath) {
	file_t romDiskFile;
	uint8* romDiskBuffer;
	long romDiskSize;

	romDiskFile = fileOpen(tFilePath, O_RDONLY);
	romDiskSize = fileTotal(romDiskFile);

	romDiskBuffer = malloc(romDiskSize);
	fileRead(romDiskFile, romDiskBuffer, romDiskSize);
	fs_romdisk_mount(tMountPath, romDiskBuffer, 1);

	fileClose(romDiskFile);
}

void unmountRomdisk(char* tMountPath) {
	fs_romdisk_unmount(tMountPath);
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

