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
#include <functional>
#include "pattern.h"

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

extern vector<Pattern> patternRules;

static class InitTokensClass {
public:
	InitTokensClass() {
		for (auto &kw: keywordNames) {
			typeNames[kw.first] = kw.second;
			for (auto &c: kw.second) {
				c = tolower(c);
			}
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
//	auto endString = "";
	auto endToken = Token::None;
	if (it->type() == Token::LeftParenthesis) {
		endToken = Token::RightParenthesis;
		++it;
	}
	for (; it != end(); ++it) {
//		cout << it->token << endl;
		if (it->token == "(") {
			groupBraces(it - begin()); //Find inner paranthesis
		}
		else if (it->type() != Token::None && it->type() == endToken) {
			group(start, it - begin() + 1, true, Token::Parenthesis);
			return;
		}
	}
	if (this->at(start) == Token::LeftParenthesis) {
		throw VerificationError(this->token, "could not find end parenthesis");
	}
}

string Group::spelling() const {
	ostringstream ss;
	ss << token.spelling();
	for (auto &c: children) {
		ss << c.spelling();
	}
	ss << endToken.spelling();
	return ss.str();
}

Token Group::concat() const {
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

Token Group::concatSmall() const {
	ostringstream ss;
	ostringstream ssCased;
	Token ret = token;
	if (!token.empty()) {
		ss << (string) token;
		ss << " ";
	}
	for (auto &c: children) {
		ss << (string) c.concatSmall();
		ss << " ";
	}
	if (!endToken.empty()) {
		ss << (string) endToken;
		ss << " ";
	}
	ret.assign(ss.str());
//	if (!ret.empty()) {
//		ret.pop_back();
//	}
	return ret;
}


void Group::groupLines() {
	if (children.size() <= 1) {
		return;
	}

	auto first = begin();
	auto it= first;

	for (;it != end(); ++it) {
		if (it->lineEnding()) {
			group(first, it + 1, false, Token::Line);
			it = first;
			++first;
		}
	}
	if (children.front().type() != Token::Line) {
		group(begin(), end(), false, Token::Line);
	}
	else if (first != end()) {
		group(first, end(), false, Token::Line);
	}
}

void Group::groupBlocks(size_t b) {
	//Todo: This function needs to be optimized for performance
	const map<Token::Type, Token::Type> blockNames {
			{Token::Class, Token::End},
			{Token::If, Token::End},
			{Token::While, Token::End},
			{Token::For, Token::Next},
			{Token::Sub, Token::End},
			{Token::Function, Token::End},
			{Token::Begin, Token::End},
			{Token::With, Token::End},
			{Token::Do, Token::Loop},
			{Token::Select, Token::End},
			{Token::Enum, Token::End},
			{Token::TypeKeyword, Token::End},
	};

	Token::Type endType = Token::None;

	auto isBlockStart = [&](Group &g) -> pair<bool, Token::Type> {
		if (g.type() != Token::Line) {
			return {false, Token::None};
		}
		if (g.empty()) {
			return {false, Token::None};
		}
		auto type = g.front().type();
		if (type >= Token::Public && type <= Token::Private) {
			if (g.size() > 1) {
				type = g[1].type();
			}
			else {
				return {false, Token::None};
			}
		}
		for (auto &bn: blockNames) {
			if (bn.first == type) {
				if (type == Token::If) {
					//Make sure that inline if statements dont start blocks
					if (g.back().type() != Token::Then) {
						return {false, Token::None};
					}
				}
				return {true, bn.second};
			}
		}
		return {false, Token::None};
	};

	auto isBlockEnd = [&](Group &g) {
		if (endType == Token::None) {
			return false;
		}
		if (g.type() == endType) {
			return true;
		}
		if(g.type() != Token::Line) {
			return false;
		}
		if (g.empty()) {
			return false;
		}
		if (g.front().type() == endType) {
			return true;
		}
		return false;
	};

	if (children.size() < 2) {
		return;
	}

	auto first = begin() + b;
	auto res = isBlockStart(*first);
	if (res.first) {
		endType = res.second;
		++first;
	}
	for (auto it=first; it != end(); ++it) {
		if (isBlockStart(*it).first) {
			groupBlocks(it - begin());
		}
		else if (endType != Token::None && isBlockEnd(*it)) {
			(begin() + b)->type(Token::BlockBegin);
			it->type(Token::BlockEnd);
			group(b, it + 1 - begin(), false, Token::Block);
			return;
		}
	}
}

void Group::groupPatterns() {
	if (size() > 1) {
		//No point in group element with only one element
		for (auto &pattern: patternRules) {
			if (size() == pattern.size() && type() == pattern.result) {
				// Ignore patterns with same result type and length (it has probably been matched already)
			}
			else if (pattern.options == pattern.FromLeft) {
				//Skip to match the first element if the pattern is the same type as this
				for (int i = (type() == pattern.result); i + pattern.size() < size() + 1; ++i) {
					// The same pattern can be matched with the same i
					// hence the while loop
					while((i + pattern.size() < size() + 1) && pattern.isMatch(i, *this)) {
						group(i, i + pattern.size(), false, pattern.result);
					}
				}
			}
			else if (pattern.options == Pattern::FromRight) {
				for (int i = - pattern.size() + size(); i >= 0; --i) {
					while ((i + pattern.size() < size() + 1) && pattern.isMatch(i, *this)) {
						group(i, i + pattern.size(), false, pattern.result);
					}
				}
			}
			else if (pattern.options == Pattern::LineRule) {
				if (size() >= pattern.size() && pattern.result != type()) {
					if (pattern.isMatch(0, *this)) {
						group(0, pattern.size(), false, pattern.result);
					}
				}
			}
		}
	}

	for (int i = 0; i < children.size(); ++i) {
		auto &c = children[i];
		c.groupPatterns();
	}
}


void Group::verify() {
	for (auto &t: children) {
		if (t.type() == Token::Block) {
			t.verify();
		}
		if (t.type() == Token::Line || t.type() == Token::BlockBegin || t.type() == Token::BlockEnd) {
			if (t.children.size() > 1) {
				stringstream ss;
				t.printRecursive(ss, 0);
				throw VerificationError(t.children[1].token, " Expected end of line. Got '" + t.children[1].token.wordSpelling() + "':\n" + t.spelling() + "\n" + ss.str());
			}
		}
	}
}

string Group::typeString() const {
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

void Group::printRecursive(std::ostream &stream, int depth) const {
	for (int i = 0; i < depth; ++i) stream << "    ";

//	stream << location();
	stream << "'" << (string) token << "' ---> (" << typeString() << ")" ;

	if (!token.cased.empty() && token.cased != token) {
		stream << " cased: '" << token.cased << "'";
	}

	stream << endl;

	for (auto &c: children) {
		c.printRecursive(stream, depth + 1);
	}

	if (!endToken.empty()) {
		for (int i = 0; i < depth; ++i) stream << "    ";
		stream << "'" << (string) endToken << "'" << endl;
	}
}

VerificationError::VerificationError(Token t, string message) {
	stringstream ss;
	ss << t.location() << ":error " << message;
	this->message = ss.str();
}

Group Group::flattenCommaList() const {
	if (type() != Token::CommaList) {
		throw VerificationError(token, " group is not a comma list");
	}
	Group ret;

	const Group *p = this;

	while (p->type() == Token::CommaList) {
		ret.children.insert(ret.begin(), p->children[2]);
		ret.children.insert(ret.begin(), p->children[1]);

		p = &p->front();
	}

	ret.children.insert(ret.begin(), *p);

	ret.type(Token::CommaList);

	return ret;
}

}  // namespace name
