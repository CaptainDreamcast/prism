#pragma once

#include "prism/file.h"

void initRomdisks();
FileHandler fileOpenRomdisk(char* tPath, int tFlags);
int fileCloseRomdisk(FileHandler tHandler);
size_t fileReadRomdisk(FileHandler tHandler, void* tBuffer, size_t tCount);
size_t fileSeekRomdisk(FileHandler tHandler, size_t tOffset, int tWhence);
size_t fileTellRomdisk(FileHandler tHandler);
size_t fileTotalRomdisk(FileHandler tHandler);

void mountRomdiskWindowsFromBuffer(Buffer b, char* tMountPath);
void mountRomdiskWindows(char* tFilePath, char* tMountPath);
void unmountRomdiskWindows(char* tMountPath);
int isRomdiskPath(char* tPath);
int isRomdiskFileHandler(FileHandler tHandler);