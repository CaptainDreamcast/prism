#ifndef TARI_FILE
#define TARI_FILE

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef struct {
  int mIsOwned;
  uint32_t mLength;
  void* mData;
} Buffer;

int fileOpen(char* tPath, int tFlags);
int fileClose(int tHandler);
size_t fileRead(int tHandler, void* tBuffer, size_t tCount);
size_t fileWrite(int tHandler, const void* tBuffer, size_t tCount);
size_t fileSeek(int tHandler, size_t tOffset, int tWhence);
size_t fileTell(int tHandler);
size_t fileTotal(int tHandler);
int fileUnlink(char* tPath);
void* fileMemoryMap(int tHandler);


Buffer fileToBuffer(char* path);
void freeBuffer(Buffer buffer);
void appendTerminationSymbolToBuffer(Buffer* tBuffer);

void initFileSystem();
void setFileSystem(char* path);
void setWorkingDirectory(char* path);

void mountRomdisk(char* tFilePath, char* tMountPath);
void unmountRomdisk(char* tMountPath);

char* getPureFileName(char* path);
char* getFileExtension(char* tPath);
void  getPathWithNumberAffixedFromAssetPath(char* tDest, char* tSrc, int i);
void getFullPath(char* tDest, char* tPath);

void printDirectory(char* tPath);

#endif
