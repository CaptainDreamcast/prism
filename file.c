#include "include/file.h"

#include <dirent.h> 

#include "include/log.h"
#include "include/memoryhandler.h"
#include "include/system.h"


static struct {
	char cwd[1024];
	char fileSystem[1024];
} gData;

char* getPureFileName(char* path){
	debugLog("Getting pure filename.");
	char* pos = strrchr(path,'/');

	if(pos == NULL) return path;
	else return pos+1;
}

char* getFileExtension(char* tPath){
	char* pos = strrchr(tPath,'.');

	if(pos == NULL) {
		logError("Unable to find file ending.");
		logErrorString(tPath);
		abortSystem();
	}

	return pos+1;
}

void getPathWithNumberAffixedFromAssetPath(char* tDest, char* tSrc, int i) {
	char* pos = strrchr(tSrc,'.');
	if(pos == NULL) {
		sprintf(tDest, "%s%d", tSrc, i);
	} else {
		pos[0] = '\0';
		sprintf(tDest, "%s%d.%s", tSrc, i, pos+1);
		pos[0] = '.';
	}

}

static int isFileMemoryMapped(file_t tFile) {
	void* data = fileMemoryMap(tFile);
	return data != NULL;
}

Buffer fileToBuffer(char* tFileDir){
	debugLog("Reading file to Buffer.");
	Buffer ret;

	size_t bufferLength;
	file_t file;
	char* data;

	file = fileOpen(tFileDir, O_RDONLY);

	if (file == FILEHND_INVALID) {
		logError("Couldn't open file.");
		logErrorString(tFileDir);
		logErrorString(gData.cwd);
		logErrorString(gData.fileSystem);
		abortSystem();
	}

	bufferLength = fileTotal(file);
	debugInteger(bufferLength);

	if(isFileMemoryMapped(file)) {
		data = fileMemoryMap(file);
		ret.mIsOwned = 0;	
	} else {
		data = allocMemory(bufferLength);
		fileRead(file, data, bufferLength);
		ret.mIsOwned = 1;
	}

	ret.mData = data;
	ret.mLength = bufferLength;

	fileClose(file);

	return ret;
}

void freeBuffer(Buffer buffer){
	debugLog("Freeing buffer.");
	if(buffer.mIsOwned){
		debugLog("Freeing owned memory");
		debugInteger(buffer.mLength);
		freeMemory(buffer.mData);
	} 
	buffer.mData = NULL;
	buffer.mLength = 0;
	buffer.mIsOwned = 0;
}

void appendTerminationSymbolToBuffer(Buffer* tBuffer) {
	if(!tBuffer->mIsOwned) {
		char* nData = allocMemory(tBuffer->mLength+1);
		memcpy(nData, tBuffer->mData, tBuffer->mLength);
		tBuffer->mData = nData;
		tBuffer->mIsOwned = 1;
	}
	else {
		tBuffer->mData = reallocMemory(tBuffer->mData, tBuffer->mLength+1);
	}


	char* buf = tBuffer->mData;
	buf[tBuffer->mLength] = '\0';
	tBuffer->mLength++;
}

void initFileSystem(){
	log("Initiate file system.");
	sprintf(gData.cwd, "/");
	gData.fileSystem[0] = '\0';
	debugString(gData.cwd);
}

void setFileSystem(char* path){
	log("Setting file system.");
	if(path[0] != '/'){
		logError("Invalid filesystem!");	
		logErrorString(path);
	}

	sprintf(gData.fileSystem, "%s", path);
	debugString(gData.fileSystem);
	
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

void getFullPath(char* tDest, char* tPath) {
	if(tPath[0] == '$') sprintf(tDest, "%s", tPath+1);
	else if(tPath[0] == '/') sprintf(tDest, "%s%s", gData.fileSystem, tPath);
	else sprintf(tDest, "%s%s%s", gData.fileSystem, gData.cwd, tPath);
}

int fileOpen(char* tPath, int tFlags){
	char path[1024];
	getFullPath(path, tPath);

	debugLog("Open file.");
	debugString(tPath);
	debugString(gData.fileSystem);
	debugString(gData.cwd);

	return fs_open(path, tFlags);
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
	struct dirent *dir;
	d = opendir(path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			logString(dir->d_name);
		}

		closedir(d);
	}
}

