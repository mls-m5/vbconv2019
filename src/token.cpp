/*
 * token.cpp
 *
 *  Created on: 14 apr. 2019
 *      Author: Mattias Lasersköld
 */


#include "token.h"
#include "file.h"
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

namespace vbconv {

#undef TYPE

#define ts(n)   {Token::n, #n},
#define tn(n)
#define tl(n, t)
#define td(n)

// Named types that are not associated to any keyword
static map<Token::Type, string> typeNames = {
		ts(Word)
		ts(None)
#include "typemacros.h"

};


#undef ts
#undef tn
#undef tl
#undef td

#define tn(n) {Token::n, #n},
#define tl(name, t) {Token::name, t},
#define td(t) // Exclude these tokens
#define ts(n) // not a keyword

//Keywords that is associated
static map<Token::Type, string> keywordNames = {
#include "typemacros.h"
};

#undef ts
#undef tn
#undef tl
#undef td


struct TokenPattern {
	TokenPattern(vector<Token::Type> t): types(t) {}
	TokenPattern(initializer_list<Token::Type> t): types(vector<Token::Type>(t)) {}
	TokenPattern(Token::Type t): types({t}) {}

	bool operator == (Token::Type t) {
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


	vector<Token::Type> types;
};

struct Pattern: public vector<TokenPattern> {
	enum Options {
		FromLeft,
		FromRight,
		LineRule, //Only match beginning of line and group the whole line
	} options;

	Pattern(vector<TokenPattern> t, Token::Type result, Options options = FromLeft, TokenPattern parent = Token::Any):
		vector<TokenPattern>(t),
		result(result),
		options(options),
		parentConstraint(parent)
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
		return true;
	}

	Token::Type result = Token::Group;
	TokenPattern parentConstraint; // If the parent needs to be a particular type
};


TokenPattern comparisonTokens = {
	Token::Equal,
	Token::NotEqual,
	Token::More,
	Token::MoreOrEqual,
	Token::Less,
	Token::LessOrEqual,
	Token::Is,
	Token::IsNot,
	Token::Like,
//	Token::TypeOf
};

vector<Pattern> patternRules = {
	{{Token::Any, Token::Exp, Token::Any}, Token::Exponentiation},
	{{Token::Any, TokenPattern({Token::Asterisks, Token::Slash}), Token::Any}, Token::MultiplicationOrDivision},
	{{Token::Any, TokenPattern({Token::Backslash}), Token::Any}, Token::IntegerDivision},
	{{Token::Any, TokenPattern({Token::Plus, Token::Minus}), Token::Any}, Token::AdditionOrSubtraction},
	{{Token::Any, comparisonTokens, Token::Any}, Token::ComparisonOperation},


	{{Token::Any, TokenPattern({Token::As}), Token::Any}, Token::AsClause},
	{{Token::Any, TokenPattern({Token::Coma}), Token::Any}, Token::ComaList},

	{{Token::Sub, Token::Word, Token::Parenthesis}, Token::SubStatement},
	{{Token::Class, Token::Word}, Token::SubStatement},
	{{Token::End, Token::Any}, Token::EndStatement},

	{{Token::Word, Token::Any}, Token::MethodCall, Pattern::FromLeft, Token::Line},
	{{Token::If, Token::Any, Token::Then}, Token::IfStatement, Pattern::FromLeft, Token::BlockBegin},
	{{Token::Elif, Token::Any, Token::Then}, Token::ElIfStatement, Pattern::FromLeft, Token::Line},
	{{Token::Else}, Token::ElseStatement, Pattern::FromLeft, Token::Line},
};

static class InitClass {
public:
	InitClass() {
		for (auto &kw: keywordNames) {
			typeNames[kw.first] = kw.second;
			kw.second[0] = tolower(kw.second[0]);
		}
	}
} initClass;


std::ostream& operator <<(std::ostream& stream, const Token& t) {
	return stream << t.location() << t.c_str();
}

std::ostream& operator <<(std::ostream& stream, const Token::Location& location) {
	if (location.file) {
		stream << location.file->filename;
	}
	else {
		stream << "unknown file";
	}
	return stream << ":" << location.line << ":"
					<< location.col << ":";
}

void Group::groupBraces(size_t start) {
	if (start >= children.size()) {
		return;
	}
	auto it = begin() + start;
	auto endString = "";
	if (*it == "(") {
		endString = ")";
		++it;
	}
	for (; it != end(); ++it) {
		cout << it->token << endl;
		if (it->token == "(") {
			groupBraces(it - begin()); //Find inner paranthesis
		}
		else if (it->token == endString) {
			group(start, it - begin() + 1, true, Token::Parenthesis);
			return;
		}
	}
}

string Group::spelling() {
	ostringstream ss;
	ss << token.spelling();
	for (auto &c: children) {
		ss << c.spelling();
	}
	ss << endToken.spelling();
	return ss.str();
}

Token Group::concat() {
	ostringstream ss;
	Token ret = token; //Copy location
	ss << token.spelling();
	for (auto &c: children) {
		ss << c.concat();
	}
	ss << endToken.spelling();
	ret.assign(ss.str()); // copy string value
	return ret;
}

Token Group::concatSmall() {
	ostringstream ss;
	Token ret = token;
	if (!token.empty()) {
		ss << (string) token << " ";
	}
	for (auto &c: children) {
		ss << (string) c.concatSmall() << " ";
	}
	if (!endToken.empty()) {
		ss << (string) endToken << " ";
	}
	ret.assign(ss.str());
	if (!ret.empty()) {
		ret.pop_back();
	}
	return ret;
}


void Group::groupLines() {
	if (children.size() <= 1) {
		return;
	}

	auto first = begin();

	for (auto it= first; it != end(); ++it) {
//		cout << "test: " << it->spelling() << endl;
		if (it->lineEnding()) {
			group(first, it + 1, false, Token::Line);
			cout << "new group: '" << first->spelling() << "'" << endl;
			it = first;
			++first;
		}
	}
}

void Group::groupBlocks(size_t b) {
	const map<Token::Type, Token::Type> blockNames {
			{Token::Class, Token::End},
			{Token::If, Token::End},
			{Token::While, Token::End},
			{Token::For, Token::End},
			{Token::Sub, Token::End},
			{Token::Function, Token::End},
//			"else",
//			"elseif",
	};

	Token::Type endString = Token::None;

	auto isBlockStart = [&](Group &g) -> pair<bool, Token::Type> {
		if (g.type() != Token::Line) {
			return {false, Token::None};
		}
		if (g.empty()) {
			return {false, Token::None};
		}
		for (auto &bn: blockNames) {
			if (bn.first == g.front().type()) {
				return {true, bn.second};
			}
		}
		return {false, Token::None};
	};
	auto isBlockEnd = [&](Group &g) {
		if (endString == Token::None) {
			return false;
		}
		if (g.type() == endString) {
			return true;
		}
		if(g.type() != Token::Line) {
			return false;
		}
		if (g.empty()) {
			return false;
		}
		if (g.front().type() == endString) {
			return true;
		}
	};

	if (children.size() < 2) {
		return;
	}

	auto first = begin() + b;
	auto res = isBlockStart(*first);
	if (res.first) {
		endString = res.second;
		++first;
	}
	for (auto it=first; it != end(); ++it) {
//		cout << "bl: " << it->spelling() << endl;
		if (isBlockStart(*it).first) {
			groupBlocks(it - begin());
		}
		else if (endString != Token::None && isBlockEnd(*it)) {
			(begin() + b)->type(Token::BlockBegin);
			it->type(Token::BlockEnd);
//			cout << "creating block from " << b << " " << " to " << it + 1 - begin() << endl;
			group(b, it + 1 - begin(), false, Token::Block);
//			cout << "new block '" << (begin() + b)->spelling() << endl;
			return;
		}
	}
}

void Group::groupPatterns() {
	for (auto &pattern: patternRules) {
		if (size() == pattern.size() && type() == pattern.result) {
			// Ignore patterns with same result type and length (it has probably been matched already)
		}
		if (size() > pattern.size()) {
			// The pattern does not fit
		}
		if (pattern.options == pattern.FromLeft) {
			//Skip to match the first element if the pattern is the same type as this
			for (int i = (type() == pattern.result); i + pattern.size() < size() + 1; ++i) {
				if(pattern.isMatch(i, *this)) {
					group(i, i + pattern.size(), false, pattern.result);
				}
			}
		}
		else if (pattern.options == Pattern::FromRight) {
			//Do this

			for (int i = - pattern.size() + size(); i >= 0; --i) {
				if (pattern.isMatch(i, *this)) {
					group(i, i + pattern.size(), false, pattern.result);
				}
			}
		}
		else if (pattern.options == Pattern::LineRule) {
			if (pattern.isMatch(0, *this)) {
				group(0, size(), false, pattern.result);
			}
		}
	}

	for (auto &c: children) {
		c.groupPatterns();
	}
}

string Group::typeString() {
	try {
		return typeNames.at(type());
	}
	catch (out_of_range &e) {
		return to_string(type()) + "(Unnamed)";
	}
}

void Group::setKeywords() {
	for (auto &c: children) {
		for (auto &kw: keywordNames) {
			if (c.token == kw.second) {
				c.type(kw.first);
				break;
			}
		}
	}
}

void Group::printRecursive(std::ostream &stream, int depth) {
	for (int i = 0; i < depth; ++i) stream << "    ";

//	stream << location();
	stream << "'" << (string) token << "' ---> (" << typeString() << ")" << endl;

	for (auto &c: children) {
		c.printRecursive(stream, depth + 1);
	}

	if (!endToken.empty()) {
		for (int i = 0; i < depth; ++i) stream << "    ";
		stream << "'" << (string) endToken << "'" << endl;
	}
}

}  // namespace name
