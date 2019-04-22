/*
 * vb6headers.h
 *
 *  Created on: 22 apr. 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once


#include <vector>
#include <memory>

template <class T>
class VBArray: std::vector<T> {
	VBArray(size_t size): std::vector(size) {}

	T &operator ()(size_t index) {
		return at(index);
	}
};

template <class T>
class VBType {
	T *operator -> () {
		return *this;
	}
};

template <class T>
class VBClass: std::enable_shared_from_this<T> {

};
