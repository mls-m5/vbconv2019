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
#include <cstdint>


namespace std {
// Without these the compiler will complain when using to_string on strings
	inline std::string to_string(const string& str) {
		return str;
	}
	inline std::string to_string(const char *c) {
		return std::string(c);
	}
}

namespace VB {
typedef double Currency;

template <class T>
class VBArray {
public:
	VBArray(size_t size): values(size + 1) {}
	VBArray() = default;

	T &operator ()(size_t index) {
		return values.at(index);
	}

	VBArray *operator ->() {
		return this;
	}

	// Notice that a VB resize is adding one extra value to what you would do in
	// c++ or some other language
	void resize(size_t size) {
		values.resize(size + 1);
	}

	std::vector<T> values;
};

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
		assign(ss.str());

		return *this;
	}

	template <typename T>
	VBString operator==(const T &value) {
		return (std::string)value == *this;
	}
};

inline double Rnd() {
	return (double) rand() / RAND_MAX;
}

inline double Timer() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


struct Date {
	// Stores the curent date as fractions of a day

	Date() {};
	Date(double v): value(v) {}
	Date(int year, int month, int day) {
		time_t zero = 0;
		auto tm = *localtime(&zero);
		value = std::mktime(&tm);
	}

	operator double() {
		return value;
	};

	friend std::ostream& operator<<(std::ostream &stream, const Date &d) {
		return stream << d.value;
	}

	friend std::istream& operator>>(std::istream &stream, Date &d) {
		return stream >> d.value;
	}

	double value = 0;
};


inline void Randomize(int number = 0) {
	if (number == 0) {
		srand(Timer());
	}
	else {
		srand(number);
	}
}

template <typename T>
inline double Cos(T val) { return cos(val); };
template <typename T>
inline double Sin(T val) { return sin(val); };
template <typename T>
inline double Sqr(T val) { return sqrt(val); };

template <typename T>
inline long Int(T i) {
	return (long) i;
}

inline std::string Chr(char c) {
	return std::string(1, c);
}


enum {
	vbBlue,
	vbWhite,
	vbRed,
	vbYellow,
	vbGreen,

	vbKeyRight,
	vbKeyLeft,
	vbKeyUp,
	vbKeyDown,

	vbKeyA,
	vbKeyD,
	vbKeyL,
	vbKeyN,
	vbKeyO,
	vbKeyS,
	vbKeyR,
	vbKeyW,
	vbKeyZ,
	vbKeyAdd,
	vbKeySubtract,

	vbKeyControl,
	vbKeySpace,
	vbKeyShift,
	vbKeyReturn,
	vbKeyEscape,
};


// These functions is to make the with statement work with both
// Shared pointers and values returned from functions

// This will match shared pointers
template <typename T>
inline T *_with_fixer(std::shared_ptr<T> i) {
    return i.get();
}

// This will match actual values
template <typename T>
inline T *_with_fixer(T &i) {
    return &i;
}

// This will match for example return values from functions
template <typename T>
inline const T* _with_fixer(const T&i) {
    return &i;
}



// Functions not implemented yet

void DoEvents()
#ifdef ENABLE_GUI
;
#else
{}
#endif

bool FileExists(VBString filename);

VBString InputBox(VBString text, VBString title = {}, VBString defaultValue = {});
void MessageBox(VBString text, VBString title = {});

} //namespace VB

using namespace VB;



template <typename T>
inline void _save(std::ostream &stream, T value) {
	stream.write((char *)&value, sizeof(value));
}

template <typename T>
inline void _load(std::istream &stream, T &value) {
	stream.read((char *)&value, sizeof(value));
}

template <typename T>
inline void _load(std::istream &stream, VBArray<T> &value) {
	short lowerBound;
	int upperBound;
	stream.read((char *)&lowerBound, sizeof(lowerBound));
	stream.read((char *)&upperBound, sizeof(upperBound));
	value.resize(upperBound - lowerBound - 1);
	for (int i = lowerBound ; i < upperBound; ++i) {
		T element;
		_load(stream, element);
		value(i - lowerBound) = element;
	}
}

inline void _save(std::ostream &stream, Currency value) {
	long long out = 1000LL * value;
	stream.write((char *)&out, sizeof(out));
}

inline void _load(std::istream &stream, Currency &value) {
	long long in = 0;
	stream.read((char *)&in, sizeof(in));
	value = (double)in / 1000.;
}
