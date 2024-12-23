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

namespace prism {

template <template <typename, typename, typename...> class MapType, class K, class V, typename Alloc, typename... Args>
void stl_new_map(MapType<K, V, Alloc, Args...>& tMap) {
	tMap.clear();
}

template <template <typename, typename, typename...> class MapType, class K, class V, typename Alloc, typename... Args>
void stl_delete_map(MapType<K, V, Alloc, Args...>& tMap) {
	tMap.clear();
}

extern int gSTLCounter;

inline int stl_int_map_get_id() {
	int id = gSTLCounter++;
	return id;
}

template <template <typename, typename, typename...> class MapType, typename T, typename Alloc, typename... Args>
int stl_int_map_push_back(MapType<int, T, Alloc, Args...>& tMap, T& tElement) {
	int id = gSTLCounter++;

	tMap[id] = tElement;
	return id;
}

template <template <typename, typename, typename...> class MapType, typename T, typename Alloc, typename... Args>
int stl_int_map_push_back(MapType<int, T, Alloc, Args...>& tMap, T&& tElement) {
	int id = gSTLCounter++;

	tMap[id] = std::move(tElement);
	return id;
}

template <template <typename, typename, typename...> class MapType, class T, class C, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(MapType<int, T, Alloc, Args...>&tMap, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename MapType<int, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
        typename MapType<int, T, Alloc, Args...>::iterator current = it;
        it++;
		int isDeleted = tFunc(tCaller, val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class T, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(MapType<int, T, Alloc, Args...>&tMap, int(*tFunc)(int tKey, T& tData)) {
	typename MapType<int, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		typename MapType<int, T, Alloc, Args...>::iterator current = it;
		it++;
		int isDeleted = tFunc(val.first, val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class T, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(MapType<int, T, Alloc, Args...>&tMap, int(*tFunc)(T& tData)) {
	typename MapType<int, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		typename MapType<int, T, Alloc, Args...>::iterator current = it;
		it++;
		int isDeleted = tFunc(val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class T, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(MapType<int, std::unique_ptr<T>, Alloc, Args...>&tMap, int(*tFunc)(T& tData)) {
	typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator current = it;
		it++;
		int isDeleted = tFunc(*val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class T, class C, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(MapType<int, std::unique_ptr<T>, Alloc, Args...>&tMap, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator current = it;
		it++;
		int isDeleted = tFunc(tCaller, *val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class O, class T, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(O& tClass, MapType<int, std::unique_ptr<T>, Alloc, Args...>&tMap, int(O::*tFunc)(T& tData)) {
	typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator current = it;
		it++;
		int isDeleted = (tClass.*tFunc)(*val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class T, typename Alloc, typename... Args>
void stl_int_map_remove_predicate(MapType<int, std::unique_ptr<T>, Alloc, Args...>&tMap, int(T::*tFunc)()) {
	typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, std::unique_ptr<T>> &val = *it;
		typename MapType<int, std::unique_ptr<T>, Alloc, Args...>::iterator current = it;
		it++;
		int isDeleted = (*val.second.*tFunc)();
		if (isDeleted) tMap.erase(current);
	}
}

template <template <typename, typename, typename...> class MapType, class T, class C, typename Alloc, typename... Args>
void stl_int_map_map(MapType<int, T, Alloc, Args...>&tMap, void(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	typename MapType<int, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		it++;
		tFunc(tCaller, val.second);
	}
}

template <template <typename, typename, typename...> class MapType, class T, typename Alloc, typename... Args>
void stl_int_map_map(MapType<int, T, Alloc, Args...>&tMap, void(*tFunc)(int tKey, T& tData)) {
	typename MapType<int, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		it++;
		tFunc(val.first, val.second);
	}
}

template <template <typename, typename, typename...> class MapType, class T, typename Alloc, typename... Args>
void stl_int_map_map(MapType<int, T, Alloc, Args...>&tMap, void(*tFunc)(T& tData)) {
	typename MapType<int, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		it++;
		tFunc(val.second);
	}
}

template <template <typename, typename, typename...> class MapType, class T, class C, typename Alloc, typename... Args>
void stl_string_map_map(MapType<std::string, T, Alloc, Args...>&tMap, void(*tFunc)(C* tCaller, const std::string &tKey, T& tData), C* tCaller = NULL) {
	typename MapType<std::string, T, Alloc, Args...>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const std::string, T> &val = *it;
		it++;
		tFunc(tCaller, val.first, val.second);
	}
}

template<template <typename, typename, typename...> class MapType, class K, class V, typename Alloc, typename... Args>
int stl_map_contains(const MapType<K, V, Alloc, Args...>& tMap, K tID)
{
	return tMap.find(tID) != tMap.end();
}

template<template <typename, typename, typename...> class MapType, class K, class V, typename Alloc, typename... Args>
std::pair<const K, V>* stl_map_get_pair_by_index(MapType<K, V, Alloc, Args...>& tMap, int tIndex)
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

template<template <typename, typename, typename...> class MapType, class V, typename Alloc, typename... Args>
int stl_string_map_contains_array(MapType<std::string, V, Alloc, Args...>& tMap, const char* tID)
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
void stl_set_map(std::set<T*> &tSet, void(*tFunc)(C* tCaller, T* tData), C* tCaller = NULL) {
	typename std::set<T*>::iterator it = tSet.begin();

	while (it != tSet.end()) {
		T *val = *it;
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
		it++;
		tFunc(tCaller, val);
	}
}

template <class T>
void stl_list_map(std::list<T> &tList, void(*tFunc)(T& tData)) {
	typename std::list<T>::iterator it = tList.begin();

	while (it != tList.end()) {
		T &val = *it;
		it++;
		tFunc(val);
	}
}

template <class T>
T& stl_list_at(std::list<T>& tList, size_t index) {
	auto it = tList.begin();
	std::advance(it, index);
	return *it;
}

}