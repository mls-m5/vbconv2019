/*
 * file.h
 *
 *  Created on: 14 apr. 2019
 *      Author: Mattias Lasersköld
 */


#pragma once

#include <token.h>
#include <memory>

namespace vbconv {

class File: std::enable_shared_from_this<File> {
	public:
	typedef std::string string;

	File(const string &filename) {
		load(filename);
	}

	void load(const string &filename);
	void tokenize();
	void group();

	auto begin() {
		return tokens.begin();
	}

	auto end() {
		return tokens.end();
	}

	string content;
	Tokens tokens;
	string filename;
};

} // namespace





