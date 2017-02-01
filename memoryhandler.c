#include "include/memoryhandler.h"

typedef struct MemoryListElement_internal {

	
	
} MemoryListElement;

typedef struct {
	int mSize;
	int mArraySize;
	MemoryListElement* mElements;
} MemoryList;

typedef struct {
	int mHead;
	MemoryList mLists[10];
} MemoryListStack;

static struct {
	MemoryListStack mMemoryStack;
	MemoryListStack mTextureMemoryStac;
} gData;


static void addMemoryToMemoryList(MemoryList* tList, void* data);
static void removeMemoryFromMemoryList(MemoryList* tList, void* data);
static void emptyMemoryList(MemoryList* tList);

static void addMemoryToMemoryListStack(MemoryList* tList, void* data);
static void removeMemoryFromMemoryListStack(MemoryList* tList, void* data);


void* allocMemory(int tSize);
void freeMemory(void* tData);
void* allocTextureMemory(int tSize);
void freeTextureMemory(void* tData);

void pushMemoryStack();
void popMemoryStack();
void pushTextureMemoryStack();
void popTextureMemoryStack();

