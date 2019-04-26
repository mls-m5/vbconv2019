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
#include <iostream>
#include <chrono>
#include <sstream>

typedef double Currency;

template <class T>
class VBArray: public std::vector<T> {
public:
	VBArray(size_t size): std::vector<T>(size) {}
	VBArray() = default;

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

class VBString: public std::string {
public:
	VBString() = default;
	VBString(const VBString&) = default;
	VBString(VBString&&) = default;
	VBString(const std::string &s): std::string(s) {}
	VBString(const char *s): std::string(s) {}
	VBString& operator=(const VBString&) = default;
	template <typename T>
	VBString(const T value) {
		std::stringstream ss;
		ss << value;
		*this = ss.str();
	}

	template <typename T>
	operator T () {
		T ret;
		std::stringstream ss(*this);
		ss >> ret;
		return ret;
	}

	template <typename T>
	T operator -(const T& value) {
		return (T)(*this) - value;
	}

	template <typename T>
	VBString &operator =(const T& value) {
		std::stringstream ss;
		ss << value;
		*this = ss.str();

		return *this;
	}

	template <typename T>
	VBString operator==(const T &value) {
		return (VBString)value == *this;
	}
};

double Rnd() {
	return (double) rand() / RAND_MAX;
}

long Timer() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void Randomize(int number = 0) {
	if (number == 0) {
		srand(Timer());
	}
	else {
		srand(number);
	}
}

#define Cos cos
#define Sin sin
#define Sqr sqrt
typedef long Date;

template <typename T>
long Int(T i) {
	return (long) i;
}

std::string Chr(char c) {
	return std::string(1, c);
}


enum {
	vbBlue,
	vbWhite,
	vbRed,
	vbYellow,

	vbKeyA,
	vbKeyW,
	vbKeyN,
	vbKeyO,
	vbKeyZ,
	vbKeyAdd,
};


template <typename T>
static void _save(std::ostream &stream, T value) {
	stream.write((char *)&value, sizeof(value));
}

template <typename T>
static void _load(std::istream &stream, T &value) {
	stream.read((char *)&value, sizeof(value));
}

static void _save(std::ostream &stream, Currency value) {
	long long out = 1000LL * value;
	stream.write((char *)&out, sizeof(out));
}

static void _load(std::istream &stream, Currency &value) {
	long long in = 0;
	stream.read((char *)&in, sizeof(in));
	value = (double)in / 1000.;
}

// Functions not implemented yet

void DoEvents() {}; //Todo: Implement

VBString InputBox(VBString text, VBString title = {}, VBString defaultValue = {});

void Cls();
void Print(VBString text);
