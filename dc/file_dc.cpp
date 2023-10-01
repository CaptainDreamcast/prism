#include "prism/file.h"

#include <dirent.h> 
#include <kos.h>

#include "prism/log.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"
#include "prism/debug.h"


static struct {
	char cwd[1024];
	char fileSystem[1024];
} gData;

void initFileSystem(){
	logg("Initiate file system.");
	setActiveFileSystemOnStartup();
	sprintf(gData.cwd, "/");
	debugString(gData.cwd);
}

void shutdownFileSystem() {}

void setFileSystem(const char* path){
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

void setWorkingDirectory(const char* path) {

	char absolutePath[1024];
	if (path[0] != '/') {
		sprintf(absolutePath, "%s%s", gData.cwd, path);
	}
	else {
		strcpy(absolutePath, path);
	}

	strcpy(gData.cwd, absolutePath);
	debugString(gPrismWindowsFileData.cwd);

	int l = int(strlen(gData.cwd));
	if (gData.cwd[l - 1] != '/') {
		gData.cwd[l] = '/';
		gData.cwd[l + 1] = '\0';
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

static FileHandler fileOpenCaseInsensitive(char* path, int tFlags) {
    DIR* d;	
    char directory[1024];
    char* folder = strrchr(path, '/');
    char* file;
    if(!folder) {
        strcpy(directory, ".");
        file = path;
    } else {
        *folder = '\0';
        strcpy(directory, path);
        *folder = '/';
        file = folder + 1;        
    }

    turnStringLowercase(file);

    FileHandler ret = FILEHND_INVALID;
	d = opendir(directory);
	if (d)
	{
		struct dirent *dir;
		while ((dir = readdir(d)) != NULL)
		{
            char testName[1024];
            strcpy(testName, dir->d_name);
			turnStringLowercase(testName);
            if(!strcmp(file, testName)) {
                sprintf(testName, "%s/%s", directory, dir->d_name);
                ret = fs_open(testName, tFlags);
                break;
            }
		}

		closedir(d);
	}

    return ret;
}

FileHandler fileOpen(const char* tPath, int tFlags){
	char path[1024];
	getFullPath(path, tPath);

	debugLog("Open file.");
	debugString(tPath);
	debugString(gData.fileSystem);
	debugString(gData.cwd);

    FileHandler ret = fs_open(path, tFlags);
    
    if(isInDevelopMode() && ret == FILEHND_INVALID) {
	    return fileOpenCaseInsensitive(path, tFlags);
    } else {
        return ret;
    }
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
void fileFlush(FileHandler /*tHandler*/) { } // Unsupported on Dreamcast
int fileUnlink(const char* tPath) {
	char path[1024];
	getFullPath(path, tPath);
	return fs_unlink(path);
}
void* fileMemoryMap(FileHandler tHandler) {
	return fs_mmap(tHandler);
}

void createDirectory(const char* tPath)
{
	if (!isDirectory(tPath)) {
		char path[1024];
		getFullPath(path, tPath);
		fs_mkdir(path);
	}
}

void fixMountPath(char* tDst, const char* tSrc) {
	if(tSrc[0] != '/') {
		sprintf(tDst, "/%s", tSrc);	
	} else {
		strcpy(tDst, tSrc);
	}
}


void mountRomdisk(const char* tFilePath, const char* tMountPath) {
	char mountPath[200];
	fixMountPath(mountPath, tMountPath);

	file_t romDiskFile;
	uint8* romDiskBuffer;
	long romDiskSize;

	romDiskFile = fileOpen(tFilePath, O_RDONLY);
	romDiskSize = fileTotal(romDiskFile);

	romDiskBuffer = (uint8*)malloc(romDiskSize);
	fileRead(romDiskFile, romDiskBuffer, romDiskSize);
	fs_romdisk_mount(mountPath, romDiskBuffer, 1);

	fileClose(romDiskFile);
}

void unmountRomdisk(const char* tMountPath) {
	char mountPath[200];
	fixMountPath(mountPath, tMountPath);

	fs_romdisk_unmount(mountPath);
}

void printDirectory(const char* tPath) {
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

