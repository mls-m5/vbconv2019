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


#define tn(n) {Token::n, #n},
#define tl(name, t) {Token::name, t},
#define td(t) //Exclude these tokens

static map<Token::Type, string> typeNames = {
		tn(Word)
		tn(Literal)
		tn(Numeric)
		tn(Operator)
		tn(Parenthesis)
		tn(Line)
		tn(Block)
		tn(BlockEnd)
		tn(BlockBegin)
		tn(None)
};

static map<Token::Type, string> keywordNames = {
#include "typemacros.h"
};

#undef tn
#undef tl
#undef td

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
			cout << "creating block from " << b << " " << " to " << it + 1 - begin() << endl;
			group(b, it + 1 - begin(), false, Token::Block);
			cout << "new block '" << (begin() + b)->spelling() << endl;
			return;
		}
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
