/*
 * token.h
 *
 *  Created on: 13 apr. 2019
 *      Author: Mattias Lasersköld
 */


#pragma once

#include <string>
#include <memory>
#include <vector>


namespace vbconv {


class Token: public std::string {
public:
	typedef std::string string;

	std::string trailingSpace;
	std::string leadingSpace;

	struct Location {
		unsigned offset = 0;
		unsigned line = 1;
		unsigned col = 1;
		class File *file;
	};

	struct Range {
		Location begin;
		Location end;
	};

	Range extent;

	void clear() {
		trailingSpace.clear();
		leadingSpace.clear();
		((string*)this)->clear();
	}

	string spelling() {
		return leadingSpace + *((string*)this) + trailingSpace;
	}

	Token() {};
	Token(const Token &) = default;
	Token(Token &&) = default;
	Token(const string str, Range extent): string(str), extent(extent) {}

	enum Type {
		Literal,
		Numeric,
		Operator,
	} type = Literal;
};

std::ostream& operator <<(std::ostream& stream, Token& t);

class Tokens {
public:
	Tokens(Token &&t): token(t) {}
	Tokens() {}

	Token token;
	std::vector<Tokens> children;

	auto begin() {
		return children.begin();
	}

	auto end() {
		return children.end();
	}

	void clear() {
		children.clear();
		token.clear();
	}

	void push_back(Token &&t) {
		children.push_back(Tokens(move(t)));
	}

	void group();

};
}  // namespace name
