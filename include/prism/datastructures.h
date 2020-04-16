#pragma once

#include <string>
#include <vector>

void turnStringLowercase(char* tString);
void turnStringLowercase(std::string& tString);
void copyStringLowercase(char* tDst, const char* tSrc);
void copyStringLowercase(std::string& tDst, const char* tSrc);
void turnStringUppercase(std::string& tString);
char* copyToAllocatedString(char* tSrc);
void removeInvalidFileNameElementsFromString(std::string& tString);

int stringBeginsWithSubstring(const char* tString, const char* tSubstring);
int stringBeginsWithSubstringCaseIndependent(const char* tString, const char* tSubstring);
int stringEqualCaseIndependent(const char* tString, const char* tOtherString);

std::vector<std::string> splitStringBySeparator(const std::string tString, char tSeparator);

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
void list_remove_predicate_inverted(List* tList, predicateCB tCB, void* tCaller);
ListIterator list_find_first_predicate(List* tList, predicateCB tCB, void* tCaller);
int list_size(List* tList);
List new_list();
void delete_list(List* tList);
ListIterator list_iterator_begin(List* tList);
ListIterator list_iterator_end(List* tList);
void* list_iterator_get(ListIterator tIterator);
void list_iterator_increase(ListIterator* tIterator);
void list_iterator_remove(List* tList, ListIterator tIterator);
int list_has_next(ListIterator tIterator);
void * list_front(List * tList);
int list_contains(List* tList, void* tData);

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
	void* mBuckets;
} StringMap;

typedef void(*stringMapMapCB)(void* caller, char* key, void* data);


StringMap new_string_map();
void delete_string_map(StringMap* tMap);
void string_map_empty(StringMap* tMap);
void string_map_push_owned(StringMap* tMap, const char* tKey, void* tData);
void string_map_push(StringMap* tMap, const char* tKey, void* tData);
void string_map_remove(StringMap* tMap, const char* tKey);
void* string_map_get(StringMap* tMap, const char* tKey);
void string_map_map(StringMap* tMap, stringMapMapCB tCB, void* tCaller);
int string_map_contains(StringMap* tMap, const char* tKey);
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

typedef struct {
	void* mData;
	int mIsOwned;
} SuffixTreeEntryData;

typedef struct SuffixTreeEntryNode_t {
	SuffixTreeEntryData* mEntry;
	struct SuffixTreeEntryNode_t* mChildren;
} SuffixTreeEntryNode;

typedef struct {
	int mSize;
	SuffixTreeEntryNode mRoot;
} SuffixTree;

SuffixTree new_suffix_tree();
SuffixTree new_suffix_tree_from_string_map(StringMap* tMap);
void delete_suffix_tree(SuffixTree* tTree);
void suffix_tree_push_owned(SuffixTree* tTree, char* tKey, void* tData);
void suffix_tree_push(SuffixTree* tTree, char* tKey, void* tData);
int suffix_tree_contains(SuffixTree* tTree, char* tKey);

void setPrismFlag(uint32_t& tFlag, uint32_t tValue);
template<class C>
inline void setPrismFlagDynamic(uint32_t& tFlag, C tValue)
{
	setPrismFlag(tFlag, uint32_t(tValue));
}
void setPrismFlagConditional(uint32_t& tFlag, uint32_t tValue, int tCondition);
template<class C>
inline void setPrismFlagConditionalDynamic(uint32_t& tFlag, C tValue, int tCondition)
{
	setPrismFlagConditional(tFlag, uint32_t(tValue), tCondition);
}
void removePrismFlag(uint32_t& tFlag, uint32_t tValue);
template<class C>
inline void removePrismFlagDynamic(uint32_t& tFlag, C tValue)
{
	removePrismFlag(tFlag, uint32_t(tValue));
}
int hasPrismFlag(const uint32_t& tFlag, uint32_t tValue);
template<class C>
inline int hasPrismFlagDynamic(uint32_t& tFlag, C tValue)
{
	return hasPrismFlag(tFlag, uint32_t(tValue));
}
