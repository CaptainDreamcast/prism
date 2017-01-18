#include "include/file.h"

char* getPureFileName(char* path){
	char* pos = strrchr(path,'/');

	if(pos == NULL) return path;
	else return pos+1;
}
