#ifndef TARI_DATASTRUCTURES
#define TARI_DATASTRUCTURES

typedef struct ListElement_internal{

	int mID;
	void* mData;
	int mIsDataOwned;
	struct ListElement_internal* mPrev;
	struct ListElement_internal* mNext;

} ListElement;


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
void list_remove(List* tList, int tID);
void list_empty(List* tList);
void list_map(List* tList, mapCB tCB, void* tCaller);
void list_remove_predicate(List* tList, predicateCB tCB, void* tCaller);
int list_size(List* tList);
List new_list();
void delete_list(List* tList);

#endif
