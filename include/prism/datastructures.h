#pragma once



void turnStringLowercase(char* tString);

typedef struct ListElement_internal {

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

typedef void(*mapCB)(void* caller, void* data);
typedef int(*predicateCB)(void* caller, void* data);
typedef int(*sortCB)(void* caller, void* data1, void* data2);


int list_push_front(List* tList, void* tData);
int list_push_front_owned(List* tList, void* tData);
int list_push_back(List* tList, void* tData);
int list_push_back_owned(List* tList, void* tData);
void* list_get(List* tList, int tID);
void* list_get_by_ordered_index(List* tList, int tIndex);
void list_remove(List* tList, int tID);
void list_empty(List* tList);
void list_map(List* tList, mapCB tCB, void* tCaller);
void list_remove_predicate(List* tList, predicateCB tCB, void* tCaller);
int list_size(List* tList);
List new_list();
void delete_list(List* tList);
ListIterator list_iterator_begin(List* tList);
void* list_iterator_get(ListIterator tIterator);
void list_iterator_increase(ListIterator* tIterator);
void list_iterator_remove(List* tList, ListIterator tIterator);
int list_has_next(ListIterator tIterator);
void * list_front(List * tList);

typedef struct {

	int mIsOwned;
	void* mData;

} VectorElement;

typedef struct {
	int mSize;
	int mAlloc;
	VectorElement* mData;
} Vector;


Vector new_vector();
Vector new_vector_with_allocated_size(int tSize);
void delete_vector(Vector* tVector);
void vector_empty(Vector* tVector);
void vector_empty_to_previous_size(Vector* tVector);
void vector_push_back(Vector* tVector, void* tData);
void vector_push_back_owned(Vector* tVector, void* tData);
void* vector_push_back_new_buffer(Vector* tVector, int tSize);
void* vector_get(Vector* tVector, int tIndex);
void vector_remove(Vector* tVector, int tIndex);
int vector_size(Vector* tVector);
void vector_sort(Vector* tVector, sortCB tCB, void* tCaller);
void vector_map(Vector* tVector, mapCB tCB, void* tCaller);
void* vector_get_back(Vector* tVector);
void vector_pop_back(Vector* tVector);


typedef struct {
	int mSize;
	void* mMap;
} StringMap;

typedef void(*stringMapMapCB)(void* caller, char* key, void* data);


StringMap new_string_map();
void delete_string_map(StringMap* tMap);
void string_map_empty(StringMap* tMap);
void string_map_push_owned(StringMap* tMap, char* tKey, void* tData);
void string_map_push(StringMap* tMap, char* tKey, void* tData);
void string_map_remove(StringMap* tMap, char* tKey);
void* string_map_get(StringMap* tMap, char* tKey);
void string_map_map(StringMap* tMap, stringMapMapCB tCB, void* tCaller);
int string_map_contains(StringMap* tMap, char* tKey);
int string_map_size(StringMap* tMap);


typedef StringMap IntMap;

IntMap new_int_map();
void delete_int_map(IntMap* tMap);
void int_map_empty(IntMap* tMap);
void int_map_push_owned(IntMap* tMap, int tKey, void* tData);
int int_map_push_back_owned(IntMap* tMap, void* tData);
void int_map_push(IntMap* tMap, int tKey, void* tData);
int int_map_push_back(IntMap* tMap, void* tData);
void int_map_remove(IntMap* tMap, int tKey);
void* int_map_get(IntMap* tMap, int tKey);
void int_map_map(IntMap* tMap, mapCB tCB, void* tCaller);
void int_map_remove_predicate(IntMap* tMap, predicateCB tCB, void* tCaller);
int int_map_contains(IntMap* tMap, int tKey);
int int_map_size(IntMap* tMap);
