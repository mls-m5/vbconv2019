/*
 * common.h
 *
 *  Created on: 25 apr. 2019
 *      Author: mattias
 */


#pragma once

#include <string>
#include <algorithm>

inline std::string stripNonAlphaNumeric(std::string s) {
	for (int i = 0; i < s.size(); ++i) {
		if (!isalpha(s[i]) && !isdigit(s[i])) {
			return std::string(s.begin(), s.begin() + i);
		}
	}
	return s;
}

inline std::string getEnding(const std::string &filename) {
	for (int i = filename.size() - 1; i >= 0; --i) {
		if (filename[i] == '/' || filename [i] == '\\') {
			return "";
		}
		else if (filename[i] == '.') {
			return std::string(filename.begin() + (i + 1), filename.end());
		}
	}
	return "";
}

inline std::string stripEnding(const std::string &filename) {
	auto ending = getEnding(filename);
	if (ending.empty()) {
		return filename;
	}
	else {
		return std::string(filename.begin(), filename.end() - ending.size() - 1);
	}
}

inline std::string getFileName(const std::string &path) {
	for (int i = path.size() - 1; i >= 0; --i) {
		if (path[i] == '/' || path [i] == '\\') {
			return std::string(path.begin() + i + 1, path.end());
		}
	}
	return path;
}

inline std::string getDirectory(const std::string &path) {
	for (int i = path.size() - 1; i >= 0; --i) {
		if (path[i] == '/' || path [i] == '\\') {
			return std::string(path.begin(), path.begin() + i) + "/";
		}
	}
	return "";
}

inline std::string toLower(std::string str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}


inline std::string getUnitName(const std::string &filename) {
	auto shortFilename = getFileName(filename);
	auto ending = getEnding(shortFilename);
	if (ending.empty()) {
		return shortFilename;
	}
	else {
		return std::string(shortFilename.begin(), shortFilename.end() - ending.size() - 1);
	}
}


template <typename T>
std::string join(T v) {
	int size = 0;
	for (auto &s: v) {
		size += s.size();
	}
	std::string ret;
	ret.reserve(size);
	for (auto &s: v) {
		ret.push_back(s);
	}
	return ret;
}
