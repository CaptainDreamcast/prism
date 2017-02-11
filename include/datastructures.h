#ifndef TARI_DATASTRUCTURES
#define TARI_DATASTRUCTURES

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


int list_push_front(List* tList, void* tData);
int list_push_front_owned(List* tList, void* tData);
int list_push_back(List* tList, void* tData);
int list_push_back_owned(List* tList, void* tData);
void* list_get(List* tList, int tID);
void list_remove(List* tList, int tID);
void list_empty(List* tList);
void list_map(List* tList, mapCB tCB, void* tCaller);
void list_remove_predicate(List* tList, predicateCB tCB, void* tCaller);
int list_size(List* tList);
List new_list();
void delete_list(List* tList);
ListIterator list_iterator_begin(List* tList);
void* list_iterator_get(ListIterator tIterator);
void list_iterator_increase(ListIterator tIterator);
int list_has_next(ListIterator tIterator);

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
void delete_vector(Vector* tVector);
void vector_empty(Vector* tVector);
void vector_push_back(Vector* tVector, void* tData);
void vector_push_back_owned(Vector* tVector, void* tData);
void* vector_get(Vector* tVector, int tIndex);
void vector_remove(Vector* tVector, int tIndex);
void* vector_pop_back(Vector* tVector);
int vector_size(Vector* tVector);


#endif
