/*
 * vb6headers.h
 *
 *  Created on: 22 apr. 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once


#include <vector>
#include <memory>

typedef double Currency;

template <class T>
class VBArray: std::vector<T> {
	VBArray(size_t size): std::vector(size) {}

	T &operator ()(size_t index) {
		return at(index);
	}

	VBArray *operator ->() {
		return this;
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

	template <typename T>
	static void _save(std::ostream &stream, T value) {
		stream.write((char *)&value, sizeof(value));
	}

	template <typename T>
	static void _load(std::istream &stream, T &value) {
		stream.read((char *)&value, sizeof(value));
	}

	static void _save(std::ostream &stream, T value) {
		long long out = 1000LL * value;
		stream.write((char *)&out, sizeof(out));
	}

	static void _load(std::istream &stream, Currency &value) {
		long long in = 0;
		stream.read((char *)&in, sizeof(in));
		value = (double)in / 1000.;
	}
};


double Rnd() {
	return (double) rand() / RAND_MAX;
}


