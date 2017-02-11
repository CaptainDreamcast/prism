#include "include/file.h"

#include "include/log.h"
#include "include/memoryhandler.h"
#include "include/system.h"


static struct {
	char cwd[100];
	char fileSystem[10];
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
		sprintf(tDest, "%s%d%s", tSrc, i, pos+1);
		pos[0] = '.';
	}

}

Buffer fileToBuffer(char* tFileDir){
	debugLog("Reading file to Buffer.");
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

	ret.mIsOwned = 0;
	ret.mData = mipMapData;
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

int fileOpen(char* tPath, int tFlags){
	char path[100];
	if(tPath[0] == '$') sprintf(path, "%s", tPath+1);
	else if(tPath[0] == '/') sprintf(path, "%s%s", gData.fileSystem, tPath);
	else sprintf(path, "%s%s%s", gData.fileSystem, gData.cwd, tPath);

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

