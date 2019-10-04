#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <memory>
#include <functional>

// until we use c++14 with DC, define make_unique ourselves
#ifdef DREAMCAST
namespace std {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#endif

template <class K, class V>
void stl_new_map(std::map<K, V>& tMap) {
	tMap.clear();
}

template <class K, class V>
void stl_delete_map(std::map<K, V>& tMap) {
	tMap.clear();
}

extern int gSTLCounter;

inline int stl_int_map_get_id() {
	int id = gSTLCounter++;
	return id;
}

template <class T>
int stl_int_map_push_back(std::map<int, T> &tMap, T& tElement) {
	int id = gSTLCounter++;

	tMap[id] = tElement;
	return id;
}

template <class T>
int stl_int_map_push_back(std::map<int, T> &tMap, T&& tElement) {
	int id = gSTLCounter++;

	tMap[id] = std::move(tElement);
	return id;
}

template <class T, class C>
void stl_int_map_remove_predicate(std::map<int, T> &tMap, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::map<int, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
        typename std::map<int, T>::iterator current = it;
        it++;
		int isDeleted = tFunc(tCaller, val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <class T>
void stl_int_map_remove_predicate(std::map<int, T> &tMap, int(*tFunc)(T& tData)) {
	typename std::map<int, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		typename std::map<int, T>::iterator current = it;
		it++;
		int isDeleted = tFunc(val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <class T>
void stl_int_map_remove_predicate(std::map<int, std::unique_ptr<T>> &tMap, int(*tFunc)(T& tData)) {
	typename std::map<int, std::unique_ptr<T>>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename std::map<int, std::unique_ptr<T>>::iterator current = it;
		it++;
		int isDeleted = tFunc(*val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <class T, class C>
void stl_int_map_remove_predicate(std::map<int, std::unique_ptr<T>> &tMap, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::map<int, std::unique_ptr<T>>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename std::map<int, std::unique_ptr<T>>::iterator current = it;
		it++;
		int isDeleted = tFunc(tCaller, *val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <class O, class T>
void stl_int_map_remove_predicate(O& tClass, std::map<int, std::unique_ptr<T>> &tMap, int(O::*tFunc)(T& tData)) {
	typename std::map<int, std::unique_ptr<T>>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename std::map<int, std::unique_ptr<T>>::iterator current = it;
		it++;
		int isDeleted = (tClass.*tFunc)(*val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <class T>
void stl_int_map_remove_predicate(std::map<int, std::unique_ptr<T>> &tMap, int(T::*tFunc)()) {
	typename std::map<int, std::unique_ptr<T>>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename std::map<int, std::unique_ptr<T>>::iterator current = it;
		it++;
		int isDeleted = (*val.second.*tFunc)();
		if (isDeleted) tMap.erase(current);
	}
}

template <class T, class C>
void stl_int_map_map(std::map<int, T> &tMap, void(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::map<int, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		it++;
		tFunc(tCaller, val.second);
	}
}

template <class T>
void stl_int_map_map(std::map<int, T> &tMap, void(*tFunc)(T& tData)) {
	typename std::map<int, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		it++;
		tFunc(val.second);
	}
}

template <class T, class C>
void stl_string_map_map(std::map<std::string, T> &tMap, void(*tFunc)(C* tCaller, const std::string &tKey, T& tData), C* tCaller = NULL) {
	typename std::map<std::string, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const std::string, T> &val = *it;
		it++;
		tFunc(tCaller, val.first, val.second);
	}
}

template <class T, class C>
void stl_string_map_map(std::unordered_map<std::string, T> &tMap, void(*tFunc)(C* tCaller, const std::string &tKey, T& tData), C* tCaller = NULL) {
	typename std::unordered_map<std::string, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const std::string, T> &val = *it;
		it++;
		tFunc(tCaller, val.first, val.second);
	}
}

template<class K, class V>
int stl_map_contains(std::map<K, V>& tMap, K tID)
{
	return tMap.find(tID) != tMap.end();
}

template<class K, class V>
int stl_map_contains(std::unordered_map<K, V>& tMap, K tID)
{
	return tMap.find(tID) != tMap.end();
}

template<class K, class V>
std::pair<const K, V>* stl_map_get_pair_by_index(std::map<K, V>& tMap, int tIndex)
{
	if (tIndex >= (int)tMap.size()) return NULL;

	auto it = tMap.begin();
	while (tIndex && it != tMap.end()) {
		it++;
		tIndex--;
	}
	if (it == tMap.end()) return NULL;

	return &(*it);
}

template<class V>
int stl_string_map_contains_array(std::map<std::string, V>& tMap, const char* tID)
{
	return tMap.find(tID) != tMap.end();
}

template<class T>
int stl_set_contains(std::set<T>& tSet, T tID)
{
	return tSet.find(tID) != tSet.end();
}

template <class T, class C>
void stl_set_map(std::set<T> &tSet, void(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::set<T>::iterator it = tSet.begin();

	while (it != tSet.end()) {
		T &val = *it;
		it++;
		tFunc(tCaller, val);
	}
}

template <class T, class C>
void stl_set_remove_predicate(std::set<T> &tSet, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::set<T>::iterator it = tSet.begin();

	while (it != tSet.end()) {
		T &val = *it;
		typename std::set<T>::iterator current = it;
		it++;
		int isDeleted = tFunc(tCaller, val);
		if (isDeleted) tSet.erase(current);
	}
}

template <class C>
void stl_new_vector(std::vector<C>& tVector) {
	tVector.clear();
}


template <class C>
void stl_delete_vector(std::vector<C>& tVector) {
	tVector.clear();
}

template <class T, class C>
void stl_vector_map(std::vector<T> &tVector, void(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::vector<T>::iterator it = tVector.begin();

	while (it != tVector.end()) {
		T &val = *it;
		it++;
		tFunc(tCaller, val);
	}
}

template <class C>
void stl_new_list(std::list<C>& tList) {
	tList.clear();
}

template <class C>
void stl_delete_list(std::list<C>& tList) {
	tList.clear();
}

template <class T, class C>
void stl_list_remove_predicate(std::list<T> &tList, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::list<T>::iterator it = tList.begin();

	while (it != tList.end()) {
		T &val = *it;
		typename std::list<T>::iterator current = it;
		it++;
		int isDeleted = tFunc(tCaller, val);
		if (isDeleted) tList.erase(current);
	}
}

template <class T, class C>
void stl_list_map(std::list<T> &tList, void(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename std::list<T>::iterator it = tList.begin();

	while (it != tList.end()) {
		T &val = *it;
		typename std::list<T>::iterator current = it;
		it++;
		tFunc(tCaller, val);
	}
}

