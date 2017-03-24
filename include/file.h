#ifndef TARI_FILE
#define TARI_FILE

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "common/header.h"

#ifdef DREAMCAST

typedef int FileHandler;

#elif defined _WIN32

#define O_RDONLY	0x1
#define O_WRONLY	0x2

#define FILEHND_INVALID	0

typedef FILE* FileHandler;

#endif

typedef struct {
  int mIsOwned;
  uint32_t mLength;
  void* mData;
} Buffer;

fup FileHandler fileOpen(char* tPath, int tFlags);
fup int fileClose(FileHandler tHandler);
fup size_t fileRead(FileHandler tHandler, void* tBuffer, size_t tCount);
fup size_t fileWrite(FileHandler tHandler, const void* tBuffer, size_t tCount);
fup size_t fileSeek(FileHandler tHandler, size_t tOffset, int tWhence);
fup size_t fileTell(FileHandler tHandler);
fup size_t fileTotal(FileHandler tHandler);
fup int fileUnlink(char* tPath);
fup void* fileMemoryMap(FileHandler tHandler);


fup Buffer fileToBuffer(char* path);
fup void freeBuffer(Buffer buffer);
fup void appendTerminationSymbolToBuffer(Buffer* tBuffer);

fup void initFileSystem();
fup void setFileSystem(char* path);
fup void setWorkingDirectory(char* path);
fup const char* getFileSystem();
fup const char* getWorkingDirectory();

fup void mountRomdisk(char* tFilePath, char* tMountPath);
fup void unmountRomdisk(char* tMountPath);

fup char* getPureFileName(char* path);
fup char* getFileExtension(char* tPath);
fup void  getPathWithNumberAffixedFromAssetPath(char* tDest, char* tSrc, int i);
fup void getFullPath(char* tDest, char* tPath);

fup void printDirectory(char* tPath);

#endif
