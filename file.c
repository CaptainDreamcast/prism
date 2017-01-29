#include "include/file.h"

#include "include/log.h"


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

	ret.isOwned = 0;
	ret.data = mipMapData;
	ret.length = bufferLength;

	fileClose(file);

	return ret;
}

void freeBuffer(Buffer buffer){
	debugLog("Freeing buffer.");
	if(buffer.isOwned){
		debugLog("Freeing owned memory");
		debugInteger(buffer.length);
		free(buffer.data);
	} 
	buffer.data = NULL;
	buffer.length = 0;
	buffer.isOwned = 0;
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
