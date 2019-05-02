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
#include <algorithm>


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

template <typename T>
size_t UBound(const VBArray<T> &arr) {
	return arr.values.size();
}

template <typename T>
size_t LBound(const VBArray<T> &arr) {
	return 0;
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
		assign(ss.str());

		return *this;
	}

	template <typename T>
	VBString operator==(const T &value) {
		return (std::string)value == *this;
	}

	friend VBString Replace(VBString str, VBString find, VBString to, int start = 0, int count = 0, int /*compare*/ = 0) {
		size_t startPos = start;
		while ((startPos = str.find(startPos, find)) != std::string::npos) {
			str.replace(startPos, find.length(), to);
			startPos += to.length();
		}
		return str;
	}

	friend VBString Trim(const VBString &str) {
		int lfind;
		for (lfind = 0; lfind < str.length(); ++lfind) {
			if (!isspace(str.at(lfind))) {
				lfind = std::max(lfind - 1, 0);
				break;
			}
		}
		int rfind;
		for (rfind = str.length() - 1; rfind > lfind; --rfind) {
			if (!isspace(str.at(rfind))) {
				rfind = std::min(rfind + 1, (int)str.size());
			}
		}

		return VBString(std::string(str.begin() + lfind, str.begin() + rfind));
	}
};


bool FileExists(VBString filename);

class VBFile: public std::fstream {
public:
	VBFile(VBString filename): std::fstream(fixFilename(filename), getFlags(fixFilename(filename))) {
		if (!*this || !this->is_open()) {
			throw "could not open file " + filename;
		}
	}

	std::ios::openmode getFlags(const VBString &filename) {
		if (FileExists(filename)) {
			return std::ios::binary | std::ios::in | std::ios::out;
		}
		else {
			// std::ios::trunc removes all content in a file if it exists
			// it it necessary to use to create a file in random access mode
			return std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc;
		}
	}

	VBString fixFilename(VBString filename) {
#ifdef __WIN32__
		return filename;
#else
		std::replace(filename.begin(), filename.end(), '\\', '/');
		return filename;
#endif
	}
};

inline double Rnd() {
	return (double) rand() / RAND_MAX;
}

inline double Timer() {
	using namespace std::chrono;
	return (double)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.;
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
	vbBlue = 0x0000ff,
	vbWhite = 0xffffff,
	vbRed = 0xff0000,
	vbYellow = 0,
	vbGreen = 0x00ff00,

	vbKeyRight = 79,
	vbKeyLeft = 80,
	vbKeyUp = 82,
	vbKeyDown = 81,

	vbKeyA = 'a',
	vbKeyD = 'd',
	vbKeyL = 'l',
	vbKeyN = 'n',
	vbKeyO = 'o',
	vbKeyS = 's',
	vbKeyR = 'r',
	vbKeyW = 'w',
	vbKeyZ = 'z',
	vbKeyAdd = '+',
	vbKeySubtract = '-',

	vbKeyControl,
	vbKeySpace = ' ',
	vbKeyShift,
	vbKeyReturn = 13,
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


VBString InputBox(VBString text, VBString title = {}, VBString defaultValue = {});
void MessageBox(VBString text, VBString title = {});

} //namespace VB

using namespace VB;



template <typename T>
inline void _save(std::ostream &stream, T value) {
	if (!stream) {
		throw std::runtime_error("file is not open");
	}
	stream.write((char *)&value, sizeof(value));
}

template <typename T>
inline void _load(std::istream &stream, T &value) {
	if (!stream) {
		throw std::runtime_error("file is not open");
	}
	stream.read((char *)&value, sizeof(value));
}

template <typename T>
inline void _save(std::ostream &stream, VBArray<T> &value) {
	if (!stream) {
		throw std::runtime_error("file is not open");
	}
	short lowerBound = 1;
	int upperBound = lowerBound + value.values.size();
	stream.write((char *)&lowerBound, sizeof(lowerBound));
	stream.write((char *)&upperBound, sizeof(upperBound));
	value.resize(upperBound - lowerBound);
	for (int i = lowerBound ; i <= upperBound; ++i) {
		_save(stream, value(i - lowerBound));
	}
}
template <typename T>
inline void _load(std::istream &stream, VBArray<T> &value) {
	if (!stream) {
		throw std::runtime_error("file is not open");
	}
	short lowerBound;
	int upperBound;
//	stream.read((char *)&lowerBound, sizeof(lowerBound));
//	stream.read((char *)&upperBound, sizeof(upperBound));
	_load(stream, lowerBound);
	_load(stream, upperBound);

	int padding;
	_load(stream, padding);
//	_load(stream, padding);

	value.resize(upperBound - lowerBound);
	for (int i = lowerBound ; i <= upperBound; ++i) {
		T element;
		_load(stream, element);
		value(i - lowerBound) = element;
	}
}

inline void _save(std::ostream &stream, Currency value) {
	if (!stream) {
		throw std::runtime_error("file is not open");
	}
	long long out = 1000LL * value;
	stream.write((char *)&out, sizeof(out));
}

inline void _load(std::istream &stream, Currency &value) {
	if (!stream) {
		throw std::runtime_error("file is not open");
	}
	long long in = 0;
	stream.read((char *)&in, sizeof(in));
	value = (double)in / 1000.;
}
