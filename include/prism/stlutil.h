#pragma once

#include <map>
#include <set>
#include <vector>
#include <list>
#include <string>

template <class K, class V>
void stl_new_map(std::map<K, V>& tMap) {
	tMap.clear();
}

template <class K, class V>
void stl_delete_map(std::map<K, V>& tMap) {
	tMap.clear();
}

extern int gSTLCounter;

template <class T>
int stl_int_map_push_back(std::map<int, T> &tMap, T tElement) {
	int id = gSTLCounter++;

	tMap[id] = tElement;
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
void stl_int_map_remove_predicate(std::map<int, T> &tMap, int(*tFunc)(T& tData)) { // TODO: rewrite with lambdas
	typename std::map<int, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
		typename std::map<int, T>::iterator current = it;
		it++;
		int isDeleted = tFunc(val.second);
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
void stl_int_map_map(std::map<int, T> &tMap, void(*tFunc)(T& tData)) { // TODO: rewrite with lambdas
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

template<class K, class V>
int stl_map_contains(std::map<K, V>& tMap, K tID)
{
	return tMap.find(tID) != tMap.end();
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

