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

class File {
	public:
	typedef std::string string;

	File() {}
	File(const string &filename) {
		load(filename);
	}
	File(std::istream &stream, const string &nfilename) {
		load(stream, nfilename);
	}

	// Load a stream and set the filename to the one given
	void load(std::istream &stream, const string &filename);

	// Load a file from a filename
	void load(const string &filename);
	void tokenize();
	void groupHeader();

	auto begin() {
		return tokens.begin();
	}

	auto end() {
		return tokens.end();
	}

	string content;
	Group tokens;
	string filename;
};

} // namespace





