/*
 * vb6headers.h
 *
 *  Created on: 22 apr. 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once


#include <vector>
#include <memory>
#include <string>
#include <cmath>
#include <fstream>

typedef double Currency;

template <class T>
class VBArray: public std::vector<T> {
public:
	VBArray(size_t size): std::vector(size) {}

	T &operator ()(size_t index) {
		return this->at(index);
	}

	VBArray *operator ->() {
		return this;
	}
};

namespace std {
// Without these the compiler will complain when using to_string on strings
	std::string to_string(const string& str) {
		return str;
	}
	std::string to_string(const char *c) {
		return std::string(c);
	}
}

double Rnd() {
	return (double) rand() / RAND_MAX;
}

#define Cos cos
#define Sin sin
#define Sqr sqrt

std::string Chr(char c) {
	return std::string(1, c);
}

void DoEvents() {}; //Todo: Implement

enum {
	vbBlue,
	vbWhite,
	vbRed,

};
