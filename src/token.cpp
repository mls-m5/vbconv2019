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

static TokenPattern comparisonTokens = {
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

static TokenPattern accessSpecifierTokens = {
		Token::Public,
		Token::Private,
		Token::Protected,
};

// https://docs.microsoft.com/en-us/dotnet/visual-basic/programming-guide/language-features/data-types/type-characters
static TokenPattern typeCharacters = {
		Token::Percentage,
		Token::Et,
		Token::At,
		Token::Exclamation,
		Token::Hash,
		Token::Dollar,
};

// Possible tokens to put a - (or a single +) in front of
static TokenPattern negatable = {
		Token::Word,
		Token::Parenthesis,
		Token::UnaryIdentityOrNegation,
		Token::Numeric
};

//static TokenPattern beforeNegatable = {{}, {
//		Token::Word,
//}};

vector<Pattern> patternRules = {
	{{Token::Sub, Token::Word, Token::Parenthesis}, Token::SubStatement},
	{{Token::Class, Token::Word}, Token::SubStatement},
	{{Token::End, Token::Any}, Token::EndStatement},
	{{Token::Next, Token::Any}, Token::NextStatement},

	{{Token::Word, typeCharacters}, Token::TypeCharacterClause},
//	{{Token::Word, Token::Dot, Token::Word}, Token::PropertyAccessor, Pattern::FromRight},
	{{Token::Dot, Token::Word}, Token::PropertyAccessor, Pattern::FromRight},
	{{{Token::Word, Token::PropertyAccessor, Token::FunctionCallOrPropertyAccessor}, {Token::Parenthesis, Token::PropertyAccessor}}, Token::FunctionCallOrPropertyAccessor, Pattern::FromLeft, Token::Any, Token::SubStatement},
	{{Token::With, {Token::Word, Token::PropertyAccessor, Token::FunctionCallOrPropertyAccessor}}, Token::WithStatement},

	Pattern({{{Token::Plus, Token::Minus}, {negatable}}, Token::UnaryIdentityOrNegation, Pattern::FromRight, Token::Any, Token::None, [] (Pattern &p, int index, Group &g) {
		if (index > 0) {
			// Check so that the token before is not another operator
			if (g.type() == Token::BlockBegin || g.type() == Token::Line || g.type() == Token::BlockEnd || g.type() == Token::Root) {
				if (index == 1) {
					// Special case
					// For example "PrintNumber -1
					return true;
				}
			}
			auto typeBefore = g[index - 1].type();
			return typeBefore > Token::BinaryOperatorsBegin && typeBefore < Token::BinaryOperatorsEnd;
//			return beforeNegatable == typeBefore;

		}
		return true;
	}}),

	{{Token::Any, Token::As, Token::Any}, Token::AsClause},
	{{Token::AsClause, Token::Equal, Token::Any}, Token::DefaultAsClause},
	{{{Token::ByRef, Token::ByVal}, Token::Any}, Token::RefTypeStatement},
	{{Token::Optional, Token::Any}, Token::OptionalStatement},

	{{Token::Any, Token::Exp, Token::Any}, Token::Exponentiation},
	{{Token::Any, TokenPattern({Token::Asterisks, Token::Slash}), Token::Any}, Token::MultiplicationOrDivision},
	{{Token::Any, TokenPattern({Token::Backslash}), Token::Any}, Token::IntegerDivision},
	{{Token::Any, TokenPattern({Token::Plus, Token::Minus}), Token::Any}, Token::AdditionOrSubtraction},
	{{Token::Any, comparisonTokens, Token::Any}, Token::ComparisonOperation, Pattern::FromRight, Token::Any, Token::DefaultAsClause, [] (Pattern &p, int index, Group &g) {
		if (g[index + 1].type() == Token::Equal) {
			if (index == 0 && (g.type() == Token::Root || g.type() == Token::Line || Token::Assignment)) {
				return false; //This is a assignment
			}
			if (index > 0 && g[index - 1].type() == Token::Set) {
				return false; // This is a set operation. ie set x = object y
			}
			if (index > 2 && g[index - 1].type() == Token::Then) {
				return false; //If assignment is after inline if statement
			}
		}
		return true;
	}},
	{{Token::Not, Token::Any}, Token::LogicalNegation},
	{{Token::Any, {Token::And, Token::AndAlso}, Token::Any}, Token::Conjunction},
	{{Token::Any, {Token::Or, Token::OrElse}, Token::Any}, Token::InclusiveDisjunction},
	{{Token::Any, {Token::Xor}, Token::Any}, Token::ExclusiveDisjunction},
	{{Token::Set, Token::Any, Token::Equal, Token::Any}, Token::SetStatement, Pattern::FromLeft, Token::Any, Token::ComparisonOperation},
	{{Token::Any, Token::Equal, Token::Any}, Token::Assignment, Pattern::FromLeft, Token::Any, {Token::ComparisonOperation, Token::SetStatement, Token::DefaultAsClause}},



	{{{Token::Comma, Token::DoubleComma}, {Token::Comma}}, Token::DoubleComma, Pattern::FromLeft, Token::Any, Token::DoubleComma},
	{{Token::Any, {Token::Comma, Token::DoubleComma}, Token::Any}, Token::CommaList, Pattern::FromLeft, Token::Any, Token::DoubleComma},


	{{{Token::Word, Token::PropertyAccessor, Token::FunctionCallOrPropertyAccessor}, TokenPattern({Token::Any}, {Token::Then})}, Token::MethodCall, Pattern::FromLeft, {Token::Line, Token::Root, Token::InlineIfStatement}},
	{{Token::If, Token::Any, Token::Then, Token::Any}, Token::InlineIfStatement, Pattern::LineRule, Token::Any, {Token::IfStatement}},
	{{Token::If, Token::Any, Token::Then}, Token::IfStatement, Pattern::LineRule, Token::Token::Any, {Token::InlineIfStatement}},
	{{Token::Elseif, Token::Any, Token::Then}, Token::ElseIfStatement, Pattern::FromLeft, Token::Line},
	{{Token::Else}, Token::ElseStatement, Pattern::FromLeft, Token::Line},
	{{Token::For, Token::Any, Token::To, Token::Any, Token::Step, Token::Any}, Token::ForLoop, Pattern::LineRule, Token::Any, {Token::ForLoop}},
	{{Token::For, Token::Any, Token::To, Token::Any}, Token::ForLoop, Pattern::LineRule, Token::Any, {Token::ForLoop}},
	{{accessSpecifierTokens, Token::Any}, Token::AccessSpecifier},
	{{Token::Call, Token::Any}, Token::CallStatement},
	{{Token::Dim, {Token::CommaList, Token::AsClause, Token::DefaultAsClause, Token::Word, Token::TypeCharacterClause}}, Token::DimStatement},
	{{Token::Option, Token::Any}, Token::OptionStatement},
};

static class InitClass {
public:
	InitClass() {
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
//			"else",
//			"elseif",
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

}  // namespace name
