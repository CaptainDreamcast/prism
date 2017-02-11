#ifndef TARI_FILE
#define TARI_FILE

#include <kos.h>

#include "stdint.h"

typedef struct {
  int mIsOwned;
  uint32_t mLength;
  void* mData;
} Buffer;

int fileOpen(char* tPath, int tFlags);
#define fileRead(x,y,z) fs_read(x,y,z)
#define fileWrite(x,y,z) fs_write(x,y,z)
#define fileSeek(x,y,z) fs_seek(x,y,z)
#define fileTell(x) fs_tell(x)
#define fileTotal(x) fs_total(x)
#define fileClose(x) fs_close(x)
#define fileUnlink(x) fs_unlink(x)
#define fileMemoryMap(x) fs_mmap(x)
Buffer fileToBuffer(char* path);
void freeBuffer(Buffer buffer);
void initFileSystem();
void setFileSystem(char* path);

void mountRomdisk(char* tFilePath, char* tMountPath);
void unmountRomdisk(char* tMountPath);

char* getPureFileName(char* path);
char* getFileExtension(char* tPath);
void  getPathWithNumberAffixedFromAssetPath(char* tDest, char* tSrc, int i);

#endif
