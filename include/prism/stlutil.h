#pragma once

#include <map>

#include <prism/memoryhandler.h>

template <class K, class V>
std::map<K, V> stl_new_map() {
	std::map<K, V> ret = std::map<K, V>();
	return ret;
}

template <class K, class V>
void stl_delete_map(std::map<K, V>& tMap) {
	tMap = std::map<K, V>();
}

template <class T, class C>
void stl_int_map_remove_predicate(std::map<int, T> &tMap, int(*tFunc)(C* tCaller, T& tData), C* tCaller = NULL) {
	auto it = tMap.begin();

	while (it != tMap.end()) {
		auto &val = *it;
		int isDeleted = tFunc(tCaller, val.second);
		if (isDeleted) it = tMap.erase(it);
		else it++;
	}
}

template<class T>
int stl_int_map_contains(std::map<int, T>& tMap, int tID)
{
	return tMap.find(tID) != tMap.end();
}