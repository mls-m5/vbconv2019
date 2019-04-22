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

	string trailingSpace;
	string leadingSpace;
	string cased;

	struct Location {
		unsigned offset = 0;
		unsigned line = 1;
		unsigned col = 1;
		class File *file = 0;
	};

	Location _location;

	auto location() const {
		return _location;
	}

	void location(const Location &l) {
		_location = l;
	}

	void clear() {
		trailingSpace.clear();
		leadingSpace.clear();
		((string*)this)->clear();
	}

	// How it was originally written
	string spelling() const {
		if (cased.empty()) {
			return leadingSpace + *((string*)this) + trailingSpace;
		}
		else {
			return leadingSpace + cased + trailingSpace;
		}
	}

	// How it was written without whitespace
	string wordSpelling() const {
		if (cased.empty()) {
			return *((string*)this);
		}
		else {
			return cased;
		}
	}

	string &name() {
		return *(string*)this;
	}

	bool lineEnding() {
		if (!trailingSpace.empty()) {
			return trailingSpace.find('\n') != string::npos;
		}
		else if (!empty()) {
			return find('\n') != string::npos;
		}
		else if (!leadingSpace.empty()) {
			return leadingSpace.find('\n');
		}
		return false;
	}

	Token &assign(const string &str) {
		toLower();

		return *this;
	}

	Token() {};
	Token(const Token &) = default;
	Token(Token &&) = default;
	Token &operator = (const Token &t) = default;
	Token &operator = (Token &&t) = default;
	Token(const string str, Location location): string(str), _location(location) {}


#define tn(t) t, //Macros to process the typemacros.h
#define tl(t, s) t,
#define td(t) t,
#define ts(t) t,

	enum Type {
		Remove = -3,
		Any = -2,
		None = -1,
		Word = 0,


#include "typemacros.h"

	} type = None;

#undef tn
#undef tl
#undef td
#undef ts

	void toLower() {
		if (type == Literal) {
			return;
		}
		cased = *this;
		for (auto &c: *this) {
			c = tolower(c);
		}
	}
};


class VerificationError: public std::exception {
public:
	VerificationError(Token t, std::string message);

	virtual char const * what() const noexcept {
		return message.c_str();
	}

	std::string message;
};


std::ostream& operator <<(std::ostream& stream, const Token& t);
std::ostream& operator <<(std::ostream& stream, const Token::Location& location);

class Group {
public:
	typedef std::string string;
	Token token;
	Token endToken;
	std::vector<Group> children;

	Group() = default;
	Group(Group &&) = default;
	Group(const Group &) = default;
	Group(std::vector<Group> &&list): children(list) {
		token.location() = children.front().location();
	};

	Group(Token &&t): token(t) {}
	Group(const Token &t): token (t) {}
	Group(Token::Type t) { type(t); }

	Group& operator=(const Group& t) = default;

	Group& operator=(const Group&& t) {
		children = std::move(t.children);
		token = std::move(t.token);
		endToken = std::move(t.endToken);
		return *this;
	}

	bool operator == (const char *str) {
		return token == str;
	}

	bool operator == (const string &str) {
		return token == str;
	}

	auto begin() {
		return children.begin();
	}

	const auto begin() const {
		return children.begin();
	}

	auto end() {
		return children.end();
	}

	const auto end() const {
		return children.end();
	}

	auto size() const {
		return children.size();
	}

	auto empty() const {
		return children.empty();
	}

	auto &front() {
		return children.front();
	}

	const auto &front() const {
		return children.front();
	}

	auto &back() {
		return children.back();
	}

	auto &at(size_t index) {
		return children.at(index);
	}

	const auto &at(size_t index) const {
		return children.at(index);
	}

	const auto &back() const {
		return children.back();
	}

	Group &operator[] (size_t i) {
		return children[i];
	}

	const Group &operator[] (size_t i) const {
		return children[i];
	}

	auto type() const {
		return token.type;
	}

	void type(Token::Type t) {
		token.type = t;
	}

	void clear() {
		children.clear();
		token.clear();
	}

	void push_back(Token &&t) {
		children.push_back(Group(move(t)));
	}

	void push_back(Group &&g) {
		children.push_back(std::move(g));
	}

	void push_back(const Group &g) {
		children.push_back(g);
	}

	Group &assign(string str) {
		token.assign(str);

		return *this;
	}

	// Get the first token with the specified type
	const Group *getByType(Token::Type type, bool recursive = true) const {
		if (token.type == type) {
			return this;
		}
		if (recursive) {
			for (auto &c: children) {
				if (auto t = c.getByType(type, true)) {
					return t;
				}
			}
		}
		else {
			for (auto &c: children) {
				if (c.token.type == type) {
					return &c;
				}
				if (c.endToken.type == type) {
					return &c;
				}
			}
		}
		if (endToken.type == type) {
			return this;
		}
		return nullptr;
	}

	Group *getByType(Token::Type type, bool recursive = true) {
		// To not have to reimplement the above function
		return const_cast<Group *>(const_cast<const Group*>(this)->getByType(type, recursive));
	}


	string spelling() const;

	//Concatenate all tokens to one single token
	Token concat() const;
	Token concatSmall() const; // Reduces all spaces to only one single space

	void printRecursive(std::ostream &stream, int depth = 0) const;

	void setKeywords();

	Token::Location location() const {
		return token.location();
	}

	string typeString() const;

	bool lineEnding() {
		if (!endToken.empty()) {
			if (endToken.lineEnding()) {
				return true;
			}
		}
		else if (!children.empty()) {
			return children.back().lineEnding();
		}
		return token.lineEnding();
	}

	// Create groups according to rules
	void groupBraces(size_t start = 0);

	// Separate lines
	void groupLines();

	// Recursiely group blocks like if,.. endif etc
	void groupBlocks(size_t begin);

	//Do the more advanced pattern matching
	void groupPatterns();

	void verify(); // Check for obvious syntax errors

	void group(
			std::vector<Group>::iterator b,
			std::vector<Group>::iterator e,
			bool stripOuter = false,
			Token::Type t = Token::None)
	{
		group(b - begin(), e - begin(), stripOuter, t);
	}

	// create a group for a range
	// if t == Tokens::None the type of the first token will not change
	// otherwise the first token get the type of t
	void group(size_t b, size_t e, bool stripOuter = false, Token::Type t = Token::None) {
		if (b == e) {
			return;
		}

		//From here
		// https://stackoverflow.com/questions/15004517/moving-elements-from-stdvector-to-another-one
		Group group;
		if (stripOuter) {
			//Removes the outer tokens
			group.token = std::move((begin() + b)->token);
			group.endToken = std::move((begin() + e - 1)->token);
			group.children.insert(
					group.children.end(),
					std::make_move_iterator(begin() + b + 1),
					std::make_move_iterator(begin() + e - 1));
		}
		else {
			group.token.location((begin() + b)->token.location());
			group.children.insert(
					group.children.end(),
					std::make_move_iterator(begin() + b),
					std::make_move_iterator(begin() + e));
		}
		if (t != Token::None) {
			group.type(t);
		}

		//Save one element to insert the new one
		children.erase(children.begin() + b + 1, children.begin() + e);

		children[b] = std::move(group);
	}

	Group flattenCommaList() const;

	bool isDataType() {
		return type() > Token::TypesBegin && type() < Token::TypesEnd;
	}
};

}  // namespace name
