/*
 * pattern.h
 *
 *  Created on: 17 apr. 2019
 *      Author: mattias
 */

#pragma once

#include <vector>
#include <token.h>
#include <functional>

namespace vbconv {

struct TokenPattern {
	TokenPattern(std::vector<Token::Type> t, std::vector<Token::Type> e = {}): types(t), excludes(e) {}
	TokenPattern(std::initializer_list<Token::Type> t, std::initializer_list<Token::Type> e = {}):
		types(std::vector<Token::Type>(t)),
		excludes(std::vector<Token::Type>(e)) {}
	TokenPattern(Token::Type t): types({t}) {}

	bool operator == (Token::Type t) {
		for (auto &type: excludes) {
			if (type == t) {
				return false;
			}
		}

		if (types.front() == Token::Any) {
			return true;
		}

		for (auto &type: types) {
			if (type == t) {
				return true;
			}
		}
		return false;
	}

	bool operator != (Token::Type t) {
		return !(*this == t);
	}


	std::vector<Token::Type> types;
	std::vector<Token::Type> excludes; // Types to exclude
};

struct Pattern: public std::vector<TokenPattern> {
	enum Options {
		FromLeft,
		FromRight,
		LineRule, //Only match beginning of line and group the whole line
	} options;

	typedef std::function <bool (Pattern &, int, Group &)> CustomFunctionType;

	Pattern(
			std::vector<TokenPattern> t,
			Token::Type result,
			Options options = FromLeft,
			TokenPattern parent = Token::Any,
			TokenPattern blacklistedParent = Token::None,
			CustomFunctionType customFunction = nullptr):
		std::vector<TokenPattern>(t),
		result(result),
		options(options),
		parentConstraint(parent),
		blacklistedParent(blacklistedParent),
		customIsMatch(customFunction)
		{}

	bool isMatch(int index, Group &g) {
		for (auto patternIndex = 0; patternIndex < size(); ++patternIndex) {
			if ((*this)[patternIndex] != g.children[index + patternIndex].type()) {
				return false;
			}
		}
		if (parentConstraint != g.type()) {
			return false;
		}
		if (blacklistedParent == g.type()) {
			return false;
		}

		if (customIsMatch) {
			return customIsMatch(*this, index, g);
		}
		return true;
	}

	Pattern &customMatchFunction(CustomFunctionType &f) {
		customIsMatch = f;
		return *this;
	}

	Token::Type result = Token::Group;
	TokenPattern parentConstraint; // If the parent needs to be a particular type
	TokenPattern blacklistedParent = {Token::None}; //prevent two or more statements to create patterns from each other in infinity
	CustomFunctionType customIsMatch;

};

} //namespace
