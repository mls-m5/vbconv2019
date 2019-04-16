/*
 * file.cpp
 *
 *  Created on: 14 apr. 2019
 *      Author: Mattias Lasersköld
 */


#include "file.h"
#include <fstream>
#include <iostream>
using namespace std;

namespace vbconv {

void File::load(const string &nfilename) {
	ifstream file(nfilename);
	load(file, nfilename);
}


void File::load(std::istream &file, const string &nfilename) {
	filename = nfilename;

	// From here
	// https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
	file.seekg(0, std::ios::end);
	content.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	content.assign(
			(std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>()
	);

	tokenize();
	tokens.setKeywords();
	tokens.groupBraces();
	tokens.groupLines();
//	tokens.printRecursive(cout, 0);
	tokens.groupBlocks(0);
//	tokens.printRecursive(cout, 0);
	tokens.groupPatterns();
}

void File::tokenize() {
	Token::Location currentLocation;
	currentLocation.file = this;

	auto &offset = currentLocation.offset;

	if (content.empty()) {
		return;
	}

	auto get = [&]() -> char {
		if (offset + 1 < content.size()) {
			++offset;
			auto c = content[offset];

			if (c == '\n') {
				++currentLocation.line;
				currentLocation.col = 1;
			}
			else {
				++currentLocation.col;
			}

			return c;
		}
		else {
			return char_traits<char>::eof();
		}
	};

	auto peek = [&] () -> char {
		if (offset + 1 < content.size()) {
			return content[offset + 1];
		}
		else {
			// A long way of saying -1:
			return char_traits<char>::eof();
		}
	};

	auto isspecial = [&] (char c) {
		const string chars("+-*/=\".'()[]{}\\,!");
		return chars.find(c) != string::npos;
	};

	Token token;
	tokens.clear();

	auto appendSpaces = [&] () {
		while (isspace(peek())) {
			token.trailingSpace += get();
		}
	};

	auto newWord = [&] () {
		if (token.empty()) {
			cout << "trying to create new word but the old is empty" << endl;
		}
		token.location(currentLocation);
		tokens.push_back(move(token));

//		cout << "New token " << token << endl;
		token.clear();
	};

	char c = content.front(); //get();

	while (c != -1) {
		if (isspace(c)) {
			if (token.empty()) {
				token.leadingSpace += c;
				while (isspace(peek())) {
					token.leadingSpace += get();
				}
			}
			else {
				token.trailingSpace += c;
				while (isspace(peek())) {
					token.trailingSpace += get();
				}
			}
		}
		else if ((isdigit(c)) || (c == '.' && peek() == isdigit(c))) {
			bool decimalPoint = (c == '.');
			if (!token.empty()) {
				newWord();
			}
			token.type = Token::Numeric;
			token += c;
			auto p = peek();
			while ((isdigit(p)) || (!decimalPoint && p == '.')) {
				if (p == '.') {
					decimalPoint = true;
				}
				token += get();
				p = peek();
			}
		}
		else if (c == '"') {
			if (!token.empty()) {
				newWord();
			}
			token.type = Token::Literal;
			token += c;
			while (peek() != '"') {
				auto p = peek();
				if (p == '\n') {
					throw runtime_error("end of line before end of quote (\") " + token);
				}
				else if (p == -1) {
					throw runtime_error("reached end of file before end of quote" + token);
				}
				token += get();
			}
			token += get();
			appendSpaces();
			newWord();
		}
		else if (isspecial(c)) {
			if (!token.empty()) {
				newWord();
			}
			token.type = Token::Operator;
			token += c;
			//Visual basic only has single character operators
//			while (isspecial(peek())) {
//				token += get();
//			}
			appendSpaces();
			newWord();
		}
		else {
			token.type = Token::Word;
			if (!token.trailingSpace.empty()) {
				newWord();
			}
			token += c;
		}
		c = get();
	}

	if (!token.empty()) {
		newWord();
	}
}




}  // namespace vbconv

