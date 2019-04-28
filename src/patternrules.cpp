/*
 * patternrules.cpp
 *
 *  Created on: 26 apr. 2019
 *      Author: mattias
 */


#include "token.h"
#include "pattern.h"
#include <vector>

using namespace std;

namespace vbconv {



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
		Token::Numeric,
		Token::FunctionCallOrPropertyAccessor,
		Token::PropertyAccessor,
		Token::Literal,
};

vector<Pattern> patternRules = {
	{{Token::Sub, Token::Word, Token::Parenthesis}, Token::SubStatement},
	{{Token::Function, Token::Word, Token::Parenthesis, Token::As, Token::Any}, Token::FunctionStatement},
	{{Token::Function, Token::Word, Token::Parenthesis}, Token::FunctionStatement},
	{{Token::Class, Token::Word}, Token::SubStatement},
	{{Token::End, Token::Any}, Token::EndStatement},
	{{Token::Next, Token::Any}, Token::NextStatement},
	{{Token::Attribute, Token::Any}, Token::AttributeStatement},
	{{Token::Enum, Token::Word}, Token::EnumStatement},
	{{Token::Word, Token::Parenthesis, Token::Minus, Token::Parenthesis}, Token::LineDrawStatement, Pattern::LineRule, Token::Any, Token::None, [] (Pattern &p, int index, Group &g) {
		// Visual basic had a special statement for drawing lines
		// Why did they not just have it as a ordinary function?
		return g[0] == "line";
	}},

	{{Token::New, Token::Word}, Token::NewStatement},
	{{Token::Exit, Token::Any}, Token::ExitStatement},
	{{Token::Hash, Token::Numeric, Token::Slash, Token::Numeric, Token::Slash, Token::Numeric, Token::Hash}, Token::DateLiteral},
	{{Token::Hash, Token::Numeric, Token::Slash, Token::Numeric, Token::Slash, Token::Numeric, Token::Numeric, Token::Colon, Token::Numeric, Token::Word, Token::Hash}, Token::DateLiteral},
	{{Token::As, Token::Hash, {Token::Numeric, Token::Word}}, Token::FileNumberStatement, Pattern::FromLeft, Token::Any, Token::DateLiteral},
	{{Token::Hash, {Token::Numeric, Token::Word}}, Token::FileNumberStatement, Pattern::FromLeft, Token::Any, Token::DateLiteral},
	{{Token::Declare, {Token::Function, Token::Sub}, Token::Word, Token::Lib, Token::Literal, Token::Parenthesis, Token::As, Token::Any}, Token::DeclareStatement},

	Pattern({{Token::Word, typeCharacters}, Token::TypeCharacterClause, Pattern::FromLeft, Token::Any, Token::DateLiteral, [] (Pattern &p, int index, Group &g) {
		//Prevent match on concatenated strings
		if (index > 0) {
			auto typeBefore = g[index - 1].type();
			if (typeBefore == Token::Comma) {
//				return true; //True is possible but one more test must be made
			}
			else if (typeBefore > Token::BinaryOperatorsBegin && typeBefore < Token::BinaryOperatorsEnd) {
				return false;
			}
		}
		if (index + 2 < g.size()) {
			//This prevents this rule to match if it actually is a stringconcatenation in a comma list
			// eg DrawText 20, 10, Message & Message2
			auto typeAfter = g[index + 2].type();
			if ((typeAfter != Token::Comma) && (typeAfter != Token::Equal)) {
				return false;
			}
		}
		return true;
	}}),
	{{Token::Dot, {Token::Word, Token::Me}}, Token::PropertyAccessor},
	{{{Token::Word, Token::PropertyAccessor, Token::FunctionCallOrPropertyAccessor, Token::Me}, {Token::Parenthesis, Token::PropertyAccessor}}, Token::FunctionCallOrPropertyAccessor, Pattern::FromLeft, Token::Any, {Token::SubStatement, Token::LineDrawStatement}, [] (Pattern &p, int index, Group &g) {
		if (g[index + 1] == Token::PropertyAccessor) {
			if (!g[index].token.trailingSpace.empty()) {
				// If there is a space before it is probably a accessor to a with statement
				return false;
			}
		}
		return true;
	}},
	{{Token::With, {Token::Word, Token::PropertyAccessor, Token::FunctionCallOrPropertyAccessor}}, Token::WithStatement},
	{{Token::Select, Token::Case, Token::Any}, Token::SelectCaseStatement},
	{{Token::TypeKeyword, Token::Any}, Token::TypeStatement},
	{{Token::Case, Token::Any}, Token::CaseStatement, Pattern::FromLeft, Token::Any, Token::SelectCaseStatement},

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

	{{Token::Any, Token::As, TokenPattern({Token::Any}, {Token::FileNumberStatement})}, Token::AsClause},
	{{Token::AsClause, Token::Equal, Token::Any}, Token::DefaultAsClause},
	{{Token::TypeCharacterClause, Token::Equal, Token::Any}, Token::DefaultTypeCharacterClause},
	{{{Token::ByRef, Token::ByVal}, Token::Any}, Token::RefTypeStatement},
	{{Token::Optional, Token::Any}, Token::OptionalStatement},

	{{Token::Any, Token::Exp, Token::Any}, Token::Exponentiation},
	{{Token::Any, TokenPattern({Token::Asterisks, Token::Slash}), Token::Any}, Token::MultiplicationOrDivision, Pattern::FromLeft, Token::Any, Token::DateLiteral},
	{{Token::Any, TokenPattern({Token::Backslash}), Token::Any}, Token::IntegerDivision},
	{{Token::Any, Token::Mod, Token::Any}, Token::ModulusOperation},
	{{Token::Any, TokenPattern({Token::Plus, Token::Minus}), Token::Any}, Token::AdditionOrSubtraction, Pattern::FromLeft, Token::Any, Token::LineDrawStatement},
	{{Token::Any, Token::Et, Token::Any}, Token::StringConcatenation},
	{{Token::Any, TokenPattern({Token::ShiftLeft, Token::ShiftRight}), Token::Any}, Token::ArithmeticBitShift},
	{{Token::Any, comparisonTokens, Token::Any}, Token::ComparisonOperation, Pattern::FromRight, Token::Any, {Token::DefaultAsClause, Token::DefaultTypeCharacterClause}, [] (Pattern &p, int index, Group &g) {
		if (g[index + 1].type() == Token::Equal) {
			if (index == 0 && (g == Token::Root || g == Token::Line || g == Token::Assignment)) {
				return false; //This is a assignment
			}
			if (index > 0) {
				auto type = g[index - 1].type();
				if (type == Token::Set) {
					return false; // This is a set operation. ie set x = object y
				}
				else if (type == Token::Const) {
					return false; // in const statements
				}
				else if (type == Token::Else) {
					return false; //IN inline if-else statement
				}
				else if (type == Token::For) {
					return false;
				}
				else if (type == Token::Dim) {
					return false;
				}

				if (index > 2 && type == Token::Then) {
					return false; //If assignment is after inline if statement
				}
			}

		}
		return true;
	}},
	{{Token::Not, Token::Any}, Token::LogicalNegation},
	{{Token::Any, {Token::And, Token::AndAlso}, Token::Any}, Token::Conjunction},
	{{Token::Any, {Token::Or, Token::OrElse}, Token::Any}, Token::InclusiveDisjunction},
	{{Token::Any, {Token::Xor}, Token::Any}, Token::ExclusiveDisjunction},
	{{Token::Set, Token::Any, Token::Equal, Token::Any}, Token::SetStatement, Pattern::FromLeft, Token::Any, Token::ComparisonOperation},
	{{Token::Any, Token::Equal, Token::Any}, Token::Assignment, Pattern::FromLeft, Token::Any, {Token::ComparisonOperation, Token::SetStatement, Token::DefaultAsClause, Token::DefaultTypeCharacterClause}},


	{{{Token::Comma, Token::DoubleComma}, {Token::Comma}}, Token::DoubleComma, Pattern::FromLeft, Token::Any, Token::DoubleComma},
	{{Token::Any, {Token::Comma, Token::DoubleComma}, Token::Any}, Token::CommaList, Pattern::FromLeft, Token::Any, Token::DoubleComma},


	{{Token::Open, Token::Any, Token::For, Token::Any, Token::FileNumberStatement}, Token::OpenStatement, Pattern::LineRule, Token::Any, {Token::ForLoop}},
	{{Token::Get, Token::CommaList}, Token::GetStatement},
	{{Token::Put, Token::CommaList}, Token::PutStatement},
	{{Token::Close, Token::FileNumberStatement}, Token::CloseStatement},

	{{{Token::Word, Token::PropertyAccessor, Token::FunctionCallOrPropertyAccessor}, TokenPattern({Token::Any}, {Token::Then, Token::Else})}, Token::MethodCall, Pattern::FromLeft, {Token::Line, Token::Root, Token::InlineIfStatement, Token::InlineIfElseStatement}, {Token::OpenStatement}},
	{{Token::If, Token::Any, Token::Then, Token::Any, Token::Else, Token::Any}, Token::InlineIfElseStatement, Pattern::LineRule, Token::Any, {Token::IfStatement}},
	{{Token::If, Token::Any, Token::Then, Token::Any}, Token::InlineIfStatement, Pattern::LineRule, Token::Any, {Token::IfStatement, Token::InlineIfElseStatement}},
	{{Token::If, Token::Any, Token::Then}, Token::IfStatement, Pattern::LineRule, Token::Token::Any, {Token::InlineIfStatement, Token::InlineIfElseStatement}},
	{{Token::Elseif, Token::Any, Token::Then}, Token::ElseIfStatement, Pattern::FromLeft, Token::Line},
//	{{Token::Else}, Token::ElseStatement, Pattern::FromLeft, Token::Line},
	{{Token::For, Token::Any, Token::To, Token::Any, Token::Step, Token::Any}, Token::ForLoop, Pattern::LineRule, Token::Any, {Token::ForLoop, Token::OpenStatement}},
	{{Token::For, Token::Any, Token::To, Token::Any}, Token::ForLoop, Pattern::LineRule, Token::Any, {Token::ForLoop, Token::OpenStatement}},
	{{Token::Do, Token::While, Token::Any}, Token::DoWhileStatement},
	{{Token::Loop, Token::While, Token::Any}, Token::LoopWhileStatement},
	{{Token::Const, Token::Any}, Token::ConstStatement},
	{{accessSpecifierTokens, Token::Any}, Token::AccessSpecifier},
	{{Token::Call, Token::Any}, Token::CallStatement},
	{{Token::Dim, {Token::CommaList, Token::AsClause, Token::DefaultAsClause, Token::Word, Token::TypeCharacterClause}}, Token::DimStatement},
	{{Token::Redim, Token::FunctionCallOrPropertyAccessor}, Token::RedimStatement},
	{{Token::Option, Token::Any}, Token::OptionStatement},
};




}  // namespace vbconv


