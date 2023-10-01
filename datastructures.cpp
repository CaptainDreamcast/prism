#include "prism/datastructures.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <algorithm>

#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/math.h"

using namespace std;

void convertIntegerToStringFast(std::string& oRet, int tValue)
{
	static const char FAST_CONVERSION_DIGITS[] =
		"0001020304050607080910111213141516171819"
		"2021222324252627282930313233343536373839"
		"4041424344454647484950515253545556575859"
		"6061626364656667686970717273747576777879"
		"8081828384858687888990919293949596979899";

	static const auto BUFFER_SIZE = 22; // std::numeric_limits<unsigned long long>::digits10 + 3; // Vita hates this
	char buffer[BUFFER_SIZE];
	auto absoluteVal = static_cast<unsigned long long>(tValue);
	const auto isNegative = tValue < 0;
	if (isNegative) absoluteVal = 0 - absoluteVal;

	char* ptr = buffer + (BUFFER_SIZE - 1);
	while (absoluteVal >= 100) {
		const auto index = static_cast<unsigned>((absoluteVal % 100) * 2);
		absoluteVal /= 100;
		*--ptr = FAST_CONVERSION_DIGITS[index + 1];
		*--ptr = FAST_CONVERSION_DIGITS[index];
	}
	if (absoluteVal < 10) {
		*--ptr = static_cast<char>('0' + absoluteVal);
	}
	else
	{
		auto index = static_cast<unsigned>(absoluteVal * 2);
		*--ptr = FAST_CONVERSION_DIGITS[index + 1];
		*--ptr = FAST_CONVERSION_DIGITS[index];
	}

	if (isNegative) *--ptr = '-';
	oRet = std::string(ptr, buffer - ptr + BUFFER_SIZE - 1);
}

void convertFloatToStringFast(std::string& oRet, double tValue) {
	static const auto BUFFER_SIZE = 50;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%lf", tValue);
	oRet = buffer;
}

void turnStringLowercase(char* tString) {
	auto n = int(strlen(tString));

	char* pos = tString;
	int i;
	for (i = 0; i < n; i++) {
		*pos = (char)tolower((int)(*pos));
		pos++;
	}
}

void turnStringLowercase(string& tString)
{
	transform(tString.begin(), tString.end(), tString.begin(), [](char c) {return (char)::tolower(c); });
}

void copyStringLowercase(char* tDst, const char* tSrc) {
	auto n = int(strlen(tSrc));
	int i;
	for (i = 0; i <= n; i++) {
		tDst[i] = (char)tolower((int)(tSrc[i]));
	}
}

void copyStringLowercase(std::string& tDst, const char* tSrc)
{
	auto n = int(strlen(tSrc));
	tDst.reserve(n);
	int i;
	for (i = 0; i <= n; i++) {
		tDst.push_back((char)tolower((int)(tSrc[i])));
	}
}

void turnStringUppercase(std::string& tString)
{
	transform(tString.begin(), tString.end(), tString.begin(), [](char c) {return (char)::toupper(c); });
}

char* copyToAllocatedString(char* tSrc) {
	if (!tSrc) return NULL;

	auto len = strlen(tSrc);
	char* ret = (char*)allocMemory(int(len+1));
	strcpy(ret, tSrc);
	return ret;
}

void removeInvalidFileNameElementsFromString(std::string & tString)
{
	transform(tString.begin(), tString.end(), tString.begin(), [](char c) 
	{
		if (c >= 'a' && c <= 'z') return c;
		if (c >= 'A' && c <= 'Z') return c;
		if (c >= '0' && c <= '9') return c;
		return '_';
	});
}

int stringBeginsWithSubstring(const char * tString, const char * tSubstring)
{
	auto n = strlen(tSubstring);
	auto m = strlen(tString);
	if (m < n) return 0;

	size_t i;
	for (i = 0; i < n; i++) {
		if (tString[i] != tSubstring[i]) return 0;
	}

	return 1;
}

int stringBeginsWithSubstringCaseIndependent(const char * tString, const char * tSubstring)
{
	string s(tString);
	string substring(tSubstring);
	transform(s.begin(), s.end(), s.begin(), [](char c) {return (char)::tolower(c); });
	transform(substring.begin(), substring.end(), substring.begin(), [](char c) {return (char)::tolower(c); });
	return stringBeginsWithSubstring(s.data(), substring.data());
}

int stringEqualCaseIndependent(const char * tString, const char * tOtherString)
{
	string s(tString);
	string otherString(tOtherString);
	transform(s.begin(), s.end(), s.begin(), [](char c) {return (char)::tolower(c); });
	transform(otherString.begin(), otherString.end(), otherString.begin(), [](char c) {return (char)::tolower(c); });
	return s == otherString;
}

int isStringLowercase(const char* tString)
{
	for (size_t i = 0; tString[i]; i++) {
		if (isupper(tString[i])) return 0;
	}
	return 1;
}

vector<string> splitStringBySeparator(const string& tString, char tSeparator)
{
	vector<string> ret;
	size_t startPos = 0;
	size_t pos = 0;
	while ((pos = tString.find(tSeparator, startPos)) != string::npos) {
		auto word = tString.substr(startPos, pos - startPos);
		ret.push_back(word);
		startPos = pos + 1;
	}
	if (startPos != string::npos) {
		ret.push_back(tString.substr(startPos));
	}
	return ret;
}

static ListElement* newListElement(List* tList, void* tData, int tIsOwned) {
	ListElement* e = (ListElement*)allocMemory(sizeof(ListElement));
	e->mID = tList->mIDs++;
	e->mData = tData;
	e->mIsDataOwned = tIsOwned;
	e->mNext = NULL;
	e->mPrev = NULL;
	return e;
}

static int list_push_front_internal(List* tList, void* tData, int tIsOwned) {
	ListElement* e = newListElement(tList, tData, tIsOwned);

	if(tList->mFirst != NULL) {
		tList->mFirst->mPrev = e;
		e->mNext = tList->mFirst;
	}
	if(tList->mLast == NULL) tList->mLast = e;

	tList->mFirst = e;

	tList->mSize++;

	return e->mID;
}

int list_push_front(List* tList, void* tData) {
	return list_push_front_internal(tList, tData, 0);
}
int list_push_front_owned(List* tList, void* tData){
	return list_push_front_internal(tList, tData, 1);
}

static int list_push_back_internal(List* tList, void* tData, int tIsOwned) {
	ListElement* e = newListElement(tList, tData,  tIsOwned);

	if(tList->mLast != NULL) {
		tList->mLast->mNext = e;
		e->mPrev = tList->mLast;
	}
	if(tList->mFirst == NULL) tList->mFirst = e;

	tList->mLast = e;
	tList->mSize++;

	return e->mID;
}

int list_push_back(List* tList, void* tData) {
	return list_push_back_internal(tList, tData, 0);
}
int list_push_back_owned(List* tList, void* tData) {
	return list_push_back_internal(tList, tData, 1);
}

static void removeListElement(List* tList, ListElement* e){
	if(e == tList->mFirst) {
		tList->mFirst = e->mNext;
	}
	if(e == tList->mLast) {
		tList->mLast = e->mPrev;
	}
	if(e->mPrev != NULL) {
		e->mPrev->mNext = e->mNext;
	}
	if(e->mNext != NULL) {
		e->mNext->mPrev = e->mPrev;
	}

	if(e->mIsDataOwned) {
		freeMemory(e->mData);
	}

	freeMemory(e);
	tList->mSize--;
}

void* list_get(List* tList, int tID) {
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		if(cur->mID == tID) { 
			return cur->mData;
		}

		cur = cur->mNext;
	}

	return NULL;
}

void * list_get_by_ordered_index(List * tList, int tIndex)
{
	if (tIndex >= list_size(tList)) {
		logError("Trying to access invalid index.");
		logErrorInteger(tIndex);
		recoverFromError();
		return NULL;
	}

	int left = tIndex;
	ListElement* cur = tList->mFirst;
	while (left--) {
		cur = cur->mNext;
		assert(cur);
	}

	return cur->mData;
}

void list_remove(List* tList, int tID){
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		ListElement* next = cur->mNext;
		if(cur->mID == tID) { 
			removeListElement(tList, cur);
		}

		cur = next;
	}

}

static int trueFunction(void* tCaller, void* tData){
	(void) tCaller;
	(void) tData;
	return 1;
}

void list_empty(List* tList){
	list_remove_predicate(tList, trueFunction, NULL);
}


void list_map(List* tList, mapCB tCB, void* tCaller) {
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		ListElement* next = cur->mNext;
		tCB(tCaller, cur->mData);
		cur = next;
	}


}

void list_remove_predicate(List* tList, predicateCB tCB, void* tCaller){
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while(left--) {
		ListElement* next = cur->mNext;
		if(tCB(tCaller, cur->mData)) { 
			removeListElement(tList, cur);
		}

		cur = next;
	}
}

void list_remove_predicate_inverted(List* tList, predicateCB tCB, void* tCaller) {
	int left = tList->mSize;

	ListElement* cur = tList->mLast;
	while (left--) {
		ListElement* prev = cur->mPrev;
		if (tCB(tCaller, cur->mData)) {
			removeListElement(tList, cur);
		}

		cur = prev;
	}
}

ListIterator list_find_first_predicate(List * tList, predicateCB tCB, void * tCaller)
{
	int left = tList->mSize;

	ListElement* cur = tList->mFirst;
	while (left--) {
		ListElement* next = cur->mNext;
		if (tCB(tCaller, cur->mData)) {
			return cur;
		}

		cur = next;
	}

	return NULL;
}


int list_size(List* tList){
	return tList->mSize;
}


List new_list() {
	List l;
	l.mSize = 0;
	l.mFirst = NULL;
	l.mLast = NULL;
	l.mIDs = 0;
	return l;
}

void delete_list(List* tList){
	list_empty(tList);
}

ListIterator list_iterator_begin(List* tList) {
	return tList->mFirst;
}

ListIterator list_iterator_end(List * tList)
{
	return tList->mLast;
}

void* list_iterator_get(ListIterator tIterator) {
	return tIterator->mData;
}

void list_iterator_increase(ListIterator* tIterator) {
	if((*tIterator)->mNext == NULL) {
		logError("Trying to increase end iterator.");
		recoverFromError();
	}
	*tIterator = (*tIterator)->mNext;
}

void list_iterator_remove(List* tList, ListIterator tIterator) {
	removeListElement(tList, tIterator);
}

int list_has_next(ListIterator tIterator)  {
	return tIterator->mNext != NULL;
}

void * list_front(List * tList)
{
	return list_get_by_ordered_index(tList, 0);
}

typedef struct {
	void* mData;
	int mFound;
} ListContainsCaller;

static void compareSingleListElement(void* tCaller, void* tData) {
	ListContainsCaller* caller = (ListContainsCaller*)tCaller;
	caller->mFound |= caller->mData == tData;
}

int list_contains(List* tList, void* tData) {
	ListContainsCaller caller;
	caller.mData = tData;
	caller.mFound = 0;
	list_map(tList, compareSingleListElement, &caller);
	return caller.mFound;
}

Vector new_vector() {
	Vector ret;
	ret.mSize = 0;
	ret.mAlloc = 2;
	ret.mData = (VectorElement*)allocMemory(sizeof(VectorElement) * ret.mAlloc);
	return ret;
}

void delete_vector(Vector* tVector) {
	vector_empty(tVector);

	freeMemory(tVector->mData);
	tVector->mSize = 0;
	tVector->mAlloc = 0;
}


void vector_empty(Vector* tVector) {
	int i;
	for(i = 0; i < tVector->mSize; i++) {
		VectorElement* e = &tVector->mData[i];
		if(e->mIsOwned) freeMemory(e->mData);
	}

	tVector->mSize = 0;		
	tVector->mAlloc = 2;
	tVector->mData = (VectorElement*)reallocMemory(tVector->mData, sizeof(VectorElement)*tVector->mAlloc);
}

void vector_empty_to_previous_size(Vector* tVector) {
	int i;
	for(i = 0; i < tVector->mSize; i++) {
		VectorElement* e = &tVector->mData[i];
		if(e->mIsOwned) freeMemory(e->mData);
	}

	tVector->mAlloc = max(tVector->mSize, 2);
	tVector->mSize = 0;
	tVector->mData = (VectorElement*)reallocMemory(tVector->mData, sizeof(VectorElement)*tVector->mAlloc);
}

static void vector_push_back_internal(Vector* tVector, void* tData, int tIsOwned) {
	if(tVector->mSize == tVector->mAlloc) {
		tVector->mAlloc *= 2;
		tVector->mData = (VectorElement*)reallocMemory(tVector->mData, sizeof(VectorElement)*tVector->mAlloc);
	}

	VectorElement* e = &tVector->mData[tVector->mSize];
	e->mIsOwned = tIsOwned;
	e->mData = tData;
	tVector->mSize++;
}

void vector_push_back(Vector* tVector, void* tData) {
	vector_push_back_internal(tVector, tData, 0);
}

void vector_push_back_owned(Vector* tVector, void* tData) {
	vector_push_back_internal(tVector, tData, 1);
}

void* vector_push_back_new_buffer(Vector* tVector, int tSize) {
	char* b = (char*)allocMemory(tSize);
	vector_push_back_owned(tVector, b);
	return b;
}

void* vector_get(Vector* tVector, int tIndex) {
	return tVector->mData[tIndex].mData;
}

int vector_size(Vector* tVector) {
	return tVector->mSize;
}

void vector_remove(Vector* tVector, int ind) {
	VectorElement* e = &tVector->mData[ind];
	if (e->mIsOwned) {
		freeMemory(e->mData);
	}

	int i;
	for (i = ind; i < tVector->mSize - 1; i++) {
		tVector->mData[i] = tVector->mData[i + 1];
	}

	tVector->mSize--;
}

static sortCB gSortFunc;
static void* gSortCaller;

static int vector_sort_cmp(const void* tData1, const void* tData2) {
	const VectorElement* e1 = (VectorElement*)tData1;
	const VectorElement* e2 = (VectorElement*)tData2;

	return gSortFunc(gSortCaller, e1->mData, e2->mData);
}

void vector_sort(Vector* tVector, sortCB tCB, void* tCaller) {
	gSortCaller = tCaller;
	gSortFunc = tCB;
	qsort(tVector->mData, tVector->mSize, sizeof(VectorElement), vector_sort_cmp);
}

void vector_map(Vector* tVector, mapCB tCB, void* tCaller) {
	int i;
	for(i = 0; i < tVector->mSize; i++) {
		tCB(tCaller, tVector->mData[i].mData);
	}
}


void* vector_get_back(Vector* tVector) {
	return tVector->mData[tVector->mSize-1].mData;
}

void vector_pop_back(Vector* tVector) {
	vector_remove(tVector, tVector->mSize-1);
}




#define MAP_MODULO 211

typedef struct {
	char mKey[100];
	void* mData;
	int mIsOwned;
} StringMapBucketListEntry;

typedef struct {
	List mEntries;
} StringMapBucket;

int gDebugStringMapAmount;

StringMap new_string_map() {
	StringMap ret;
	ret.mBuckets = allocMemory(sizeof(StringMapBucket) * MAP_MODULO);

	int i;
	for (i = 0; i < MAP_MODULO; i++) {
		StringMapBucket* newBucket = &((StringMapBucket*)ret.mBuckets)[i];
		newBucket->mEntries = new_list();
	}

	gDebugStringMapAmount++;
	ret.mSize = 0;
	return ret;
}

static void string_map_remove_element(StringMap* tMap, StringMapBucketListEntry* e) {
	if (e->mIsOwned) {
		freeMemory(e->mData);
	}

	tMap->mSize--;
}

static int clearStringMapBucketEntry(void* tCaller, void* tData) {
	StringMap* map = (StringMap*)tCaller;
	string_map_remove_element(map, (StringMapBucketListEntry*)tData);

	return 1;
}

static void clearStringMapBucket(StringMap* tMap, StringMapBucket* e) {
	list_remove_predicate(&e->mEntries, clearStringMapBucketEntry, tMap);
	delete_list(&e->mEntries);
}

void delete_string_map(StringMap* tMap) {
	string_map_empty(tMap);
	freeMemory(tMap->mBuckets);
	gDebugStringMapAmount--;
}

void string_map_empty(StringMap* tMap) {
	int i;
	for (i = 0; i < MAP_MODULO; i++) {
		StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[i];
		clearStringMapBucket(tMap, bucket);
	}
}

static int getBucketIDFromString(uint8_t* tKey) {
	auto l = int(strlen((char*)tKey));
	int i;
	int base = 1;
	int offset = 0;
	for (i = 0; i < l; i++) {
		offset = (offset + (tKey[i] * base)) % MAP_MODULO;
		base = (base * 10) % MAP_MODULO;
	}
	return offset;
}

static void string_map_push_internal(StringMap* tMap, const char* tKey, void* tData, int tIsOwned) {
	int offset = getBucketIDFromString((uint8_t*)tKey);
	StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[offset];

	StringMapBucketListEntry* newEntry = (StringMapBucketListEntry*)allocMemory(sizeof(StringMapBucketListEntry));
	strcpy(newEntry->mKey, tKey);
	newEntry->mData = tData;
	newEntry->mIsOwned = tIsOwned;

	list_push_back_owned(&bucket->mEntries, newEntry);
	tMap->mSize++;
}

void string_map_push_owned(StringMap* tMap, const char* tKey, void* tData) {
	string_map_push_internal(tMap, tKey, tData, 1);
}

void string_map_push(StringMap* tMap, const char* tKey, void* tData) {
	string_map_push_internal(tMap, tKey, tData, 0);
}

void string_map_remove(StringMap* tMap, const char* tKey) {
	int offset = getBucketIDFromString((uint8_t*)tKey);
	StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[offset];

	if (!list_size(&bucket->mEntries)) {
		logError("Unable to find key in map.");
		logErrorString(tKey);
		recoverFromError();
	}
	ListIterator it = list_iterator_begin(&bucket->mEntries);
	while (1) {
		
		StringMapBucketListEntry* e = (StringMapBucketListEntry*)list_iterator_get(it);
		if (!strcmp(tKey, e->mKey)) {
			string_map_remove_element(tMap, e);
			list_iterator_remove(&bucket->mEntries, it);
			return;
		}
		
		if (!list_has_next(it)) {
			logError("Unable to find key in map.");
			logErrorString(tKey);
			recoverFromError();
		}

		list_iterator_increase(&it);
	}
}

void* string_map_get(StringMap* tMap, const char* tKey) {
	int offset = getBucketIDFromString((uint8_t*)tKey);
	StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[offset];

	if (!list_size(&bucket->mEntries)) {
		logError("Unable to find key in map.");
		logErrorString(tKey);
		recoverFromError();
	}

	ListIterator it = list_iterator_begin(&bucket->mEntries);
	while (1) {
		StringMapBucketListEntry* e = (StringMapBucketListEntry*)list_iterator_get(it);
		if (!strcmp(tKey, e->mKey)) {
			return e->mData;
		}
		
		if (!list_has_next(it)) {
			logError("Unable to find key in map.");
			logErrorString(tKey);
			recoverFromError();
		}

		list_iterator_increase(&it);
	}

}

typedef struct {
	void* mCaller;
	stringMapMapCB mCB;

} StringMapCaller;

static void string_map_map_single_list_entry(void* tCaller, void* tData) {
	StringMapCaller* caller = (StringMapCaller*)tCaller;
	StringMapBucketListEntry* e = (StringMapBucketListEntry*)tData;

	caller->mCB(caller->mCaller, e->mKey, e->mData);
}

static void string_map_map_single_bucket(StringMapCaller* tCaller, StringMapBucket* e) {
	list_map(&e->mEntries, string_map_map_single_list_entry, tCaller);
}

void string_map_map(StringMap* tMap, stringMapMapCB tCB, void* tCaller) {
	StringMapCaller caller;
	caller.mCaller = tCaller;
	caller.mCB = tCB;
	int i;
	for (i = 0; i < MAP_MODULO; i++) {
		StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[i];
		string_map_map_single_bucket(&caller, bucket);
	}
}

int string_map_contains(StringMap* tMap, const char* tKey) {
	int offset = getBucketIDFromString((uint8_t*)tKey);
	StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[offset];

	if (!list_size(&bucket->mEntries)) return 0;

	ListIterator it = list_iterator_begin(&bucket->mEntries);
	while (1) {
		StringMapBucketListEntry* e = (StringMapBucketListEntry*)list_iterator_get(it);
		if (!strcmp(tKey, e->mKey)) {
			return 1;
		}

		if (!list_has_next(it)) {
			break;
		}

		list_iterator_increase(&it);
	}

	return 0;
}

int string_map_size(StringMap* tMap) {
	return tMap->mSize;
}

IntMap new_int_map()
{
	return new_string_map();
}

void delete_int_map(IntMap * tMap)
{
	delete_string_map(tMap);
}

void int_map_empty(IntMap * tMap)
{
	string_map_empty(tMap);
}

static int gIntMapIndex;

void int_map_push_owned(IntMap * tMap, int tKey, void * tData)
{
	char str[100];
	sprintf(str, "%d", tKey);
	string_map_push_owned(tMap, str, tData);
}

int int_map_push_back_owned(IntMap * tMap, void * tData)
{
	int nkey = gIntMapIndex++;
	int_map_push_owned(tMap, nkey, tData);
	return nkey;
}

void int_map_push(IntMap * tMap, int tKey, void * tData)
{
	char str[100];
	sprintf(str, "%d", tKey);
	string_map_push(tMap, str, tData);
}

int int_map_push_back(IntMap * tMap, void * tData)
{
	int nkey = gIntMapIndex++;
	int_map_push(tMap, nkey, tData);
	return nkey;
}

void int_map_remove(IntMap * tMap, int tKey)
{
	char str[100];
	sprintf(str, "%d", tKey);
	string_map_remove(tMap, str);
}

void* int_map_get(IntMap * tMap, int tKey)
{
	char str[100];
	sprintf(str, "%d", tKey);
	return string_map_get(tMap, str);
}

typedef struct {
	void* mCaller;
	mapCB mCB;
} IntMapCaller;

static void int_map_map_single_list_entry(void* tCaller, void* tData) {
	IntMapCaller* caller = (IntMapCaller*)tCaller;
	StringMapBucketListEntry* e = (StringMapBucketListEntry*)tData;

	caller->mCB(caller->mCaller, e->mData);
}

static void int_map_map_single_bucket(IntMapCaller* tCaller, StringMapBucket* e) {
	list_map(&e->mEntries, int_map_map_single_list_entry, tCaller);
}

void int_map_map(IntMap* tMap, mapCB tCB, void* tCaller) {
	IntMapCaller caller;
	caller.mCaller = tCaller;
	caller.mCB = tCB;
	int i;
	for (i = 0; i < MAP_MODULO; i++) {
		StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[i];
		int_map_map_single_bucket(&caller, bucket);
	}
}

typedef struct {
	void* mCaller;
	predicateCB mCB;
	IntMap* mMap;
} IntPredicateCaller;

static int int_map_remove_predicate_single_list_entry(void* tCaller, void* tData) {
	IntPredicateCaller* caller = (IntPredicateCaller*)tCaller;
	StringMapBucketListEntry* e = (StringMapBucketListEntry*)tData;

	int ret = caller->mCB(caller->mCaller, e->mData);
	if (ret) {
		string_map_remove_element(caller->mMap, e);
	}
	return ret;
}

static void int_map_remove_predicate_single_bucket(IntPredicateCaller* tCaller, StringMapBucket* e) {
	list_remove_predicate(&e->mEntries, int_map_remove_predicate_single_list_entry, tCaller);
}


void int_map_remove_predicate(IntMap * tMap, predicateCB tCB, void * tCaller)
{
	IntPredicateCaller caller;
	caller.mCaller = tCaller;
	caller.mCB = tCB;
	caller.mMap = tMap;
	int i;
	for (i = 0; i < MAP_MODULO; i++) {
		StringMapBucket* bucket = &((StringMapBucket*)tMap->mBuckets)[i];
		int_map_remove_predicate_single_bucket(&caller, bucket);
	}
}

int int_map_contains(IntMap * tMap, int tKey)
{
	char str[100];
	sprintf(str, "%d", tKey);
	return string_map_contains(tMap, str);
}

int int_map_size(IntMap* tMap) {
	return string_map_size(tMap);
}

static SuffixTreeEntryNode createEmptyNode() {
	SuffixTreeEntryNode node;
	node.mEntry = NULL;
	node.mChildren = NULL;
	return node;
}


SuffixTree new_suffix_tree()
{
	SuffixTree ret;
	ret.mSize = 0;
	ret.mRoot = createEmptyNode();
	return ret;
}

static void new_suffix_tree_from_string_map_copy_single(void* tCaller, char* tKey, void* tData) {
	SuffixTree* tree = (SuffixTree*)tCaller;

	suffix_tree_push(tree, tKey, tData);

}

SuffixTree new_suffix_tree_from_string_map(StringMap * tMap)
{
	SuffixTree ret = new_suffix_tree();
	string_map_map(tMap, new_suffix_tree_from_string_map_copy_single, &ret);
	return ret;
}

#define SUFFIX_TREE_CHILDREN_AMOUNT (128 - 0x20)

static void delete_suffy_tree_recursively(SuffixTreeEntryNode* tNode) {
	if (tNode->mEntry) {
		SuffixTreeEntryData* entry = tNode->mEntry;
		if (entry->mIsOwned) {
			freeMemory(entry->mData);
		}

		freeMemory(tNode->mEntry);
	}

	if (tNode->mChildren) {
		int i;
		for (i = 0; i < SUFFIX_TREE_CHILDREN_AMOUNT; i++) {
			delete_suffy_tree_recursively(&tNode->mChildren[i]);
		}
		freeMemory(tNode->mChildren);
	}
}

void delete_suffix_tree(SuffixTree * tTree)
{
	delete_suffy_tree_recursively(&tTree->mRoot);
}

static void suffix_tree_add_data_to_node(SuffixTreeEntryNode* tNode, void* tData, int tIsOwned) {
	tNode->mEntry = (SuffixTreeEntryData*)allocMemory(sizeof(SuffixTreeEntryData));
	tNode->mEntry->mData = tData;
	tNode->mEntry->mIsOwned = tIsOwned;
}

static void suffix_tree_add_children_to_node(SuffixTreeEntryNode* tNode) {
	tNode->mChildren = (SuffixTreeEntryNode*)allocClearedMemory(SUFFIX_TREE_CHILDREN_AMOUNT, sizeof(SuffixTreeEntryNode));
}

static void suffix_tree_push_recursive(SuffixTreeEntryNode* tNode, char* tRestString, void* tData, int tIsOwned) {
	if (*tRestString == '\0') {
		suffix_tree_add_data_to_node(tNode, tData, tIsOwned);
		return;
	}

	if (!tNode->mChildren) {
		suffix_tree_add_children_to_node(tNode);
	}

	char currentChar = *tRestString;
	int index = currentChar - 0x20;

	suffix_tree_push_recursive(&tNode->mChildren[index], tRestString + 1, tData, tIsOwned);
}

static void suffix_tree_push_general(SuffixTree* tTree, char* tKey, void* tData, int tIsOwned) {
	suffix_tree_push_recursive(&tTree->mRoot, tKey, tData, tIsOwned);
}

void suffix_tree_push_owned(SuffixTree * tTree, char * tKey, void * tData)
{
	suffix_tree_push_general(tTree, tKey, tData, 1);
}

void suffix_tree_push(SuffixTree * tTree, char * tKey, void * tData)
{
	suffix_tree_push_general(tTree, tKey, tData, 0);
}

static void* suffix_tree_find_recursive(SuffixTreeEntryNode* tNode, char* tRestString) {
	if (*tRestString == '\0') {
		return (tNode->mEntry) ? tNode->mEntry->mData : NULL;
	}

	if (!tNode->mChildren) {
		return NULL;
	}

	char currentChar = *tRestString;
	int index = currentChar - 0x20;
	return suffix_tree_find_recursive(&tNode->mChildren[index], tRestString + 1);
}

int suffix_tree_contains(SuffixTree * tTree, char * tKey)
{
	void* data = suffix_tree_find_recursive(&tTree->mRoot, tKey);
	return data != NULL;
}

void setPrismFlag(uint32_t& tFlag, uint32_t tValue) { 
	tFlag |= tValue; 
}

void setPrismFlagConditional(uint32_t& tFlag, uint32_t tValue, int tCondition)
{
	if (tCondition) {
		setPrismFlag(tFlag, tValue);
	}
}

void removePrismFlag(uint32_t& tFlag, uint32_t tValue) {
	tFlag &= ~tValue;
}

int hasPrismFlag(const uint32_t& tFlag, uint32_t tValue) { 
	return tFlag & tValue;
}
