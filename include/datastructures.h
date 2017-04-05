#ifndef TARI_DATASTRUCTURES
#define TARI_DATASTRUCTURES

#include "common/header.h"

typedef struct ListElement_internal{

	int mID;
	void* mData;
	int mIsDataOwned;
	struct ListElement_internal* mPrev;
	struct ListElement_internal* mNext;

} ListElement;

typedef ListElement* ListIterator;


typedef struct {
	ListElement* mFirst;
	ListElement* mLast;
	int mSize;
	int mIDs;
} List;

typedef void (*mapCB)(void* caller, void* data);
typedef int (*predicateCB)(void* caller, void* data);
typedef int(*sortCB)(void* caller, void* data1, void* data2);


fup int list_push_front(List* tList, void* tData);
fup int list_push_front_owned(List* tList, void* tData);
fup int list_push_back(List* tList, void* tData);
fup int list_push_back_owned(List* tList, void* tData);
fup void* list_get(List* tList, int tID);
fup void list_remove(List* tList, int tID);
fup void list_empty(List* tList);
fup void list_map(List* tList, mapCB tCB, void* tCaller);
fup void list_remove_predicate(List* tList, predicateCB tCB, void* tCaller);
fup int list_size(List* tList);
fup List new_list();
fup void delete_list(List* tList);
fup ListIterator list_iterator_begin(List* tList);
fup void* list_iterator_get(ListIterator tIterator);
fup void list_iterator_increase(ListIterator* tIterator);
fup int list_has_next(ListIterator tIterator);

typedef struct {

	int mIsOwned;
	void* mData;

} VectorElement;

typedef struct {
	int mSize;
	int mAlloc;	
	VectorElement* mData;
} Vector;


fup Vector new_vector();
fup void delete_vector(Vector* tVector);
fup void vector_empty(Vector* tVector);
fup void vector_push_back(Vector* tVector, void* tData);
fup void vector_push_back_owned(Vector* tVector, void* tData);
fup void* vector_get(Vector* tVector, int tIndex);
fup void vector_remove(Vector* tVector, int tIndex);
fup void* vector_pop_back(Vector* tVector);
fup int vector_size(Vector* tVector);
fup void vector_sort(Vector* tVector, sortCB tCB, void* tCaller);
fup void vector_map(Vector* tVector, mapCB tCB, void* tCaller);


typedef struct {
	int mSize;
	Vector mBuckets;
} StringMap;

typedef void(*stringMapMapCB)(void* caller, char* key, void* data);

fup StringMap new_string_map();
fup void destroy_string_map(StringMap* tMap);
fup void string_map_push_owned(StringMap* tMap, char* tKey, void* tData);
fup void* string_map_get(StringMap* tMap, char* tKey);
fup void string_map_map(StringMap* tMap, stringMapMapCB tCB, void* tCaller);



#endif
