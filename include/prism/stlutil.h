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
	typename std::map<int, T>::iterator it = tMap.begin();

	while (it != tMap.end()) {
		std::pair<const int, T> &val = *it;
        typename std::map<int, T>::iterator current = it;
        it++;
		int isDeleted = tFunc(tCaller, val.second);
		if (isDeleted) tMap.erase(current);
	}
}

template<class T>
int stl_int_map_contains(std::map<int, T>& tMap, int tID)
{
	return tMap.find(tID) != tMap.end();
}
