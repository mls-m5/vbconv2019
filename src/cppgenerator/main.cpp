/*
 * main.cpp
 *
 *  Created on: 19 apr. 2019
 *      Author: mattias
 */

#include "typelibrary.h"
#include <file.h>
#include <iostream>
#include <map>
#include <functional>
#include <sstream>
#include <algorithm>

using namespace std;

namespace vbconv {

Group generateCpp(const Group &g);

static int depth = 1;
static int tabSize = 4;

struct {
	string name;
	TypeDeclaration type;
} currentFunction;

string getIndent(int d = -1) {
	if (d < 0) {
		if(depth >= 0) {
			d = depth;
		}
		else {
			d = 0;
		}
	}

	return string(d * tabSize, ' ');
}

class GenerateError: public exception {
public:
	GenerateError(const Group &g, string message) {
		stringstream ss;
		ss << g.token.location() << ":" << message << "\n" << g.spelling();
		message = ss.str();
	}

	const char *what() const noexcept override {
		return message.c_str();
	}

	string message;
};

void ExpectSize(const Group &g, int size) {
	if (g.size() != size) {
		throw GenerateError(g, "wrong size in expression " + g.typeString());
	}
}



map<Token::Type, function<Group(const Group &)>> genMap = {
		{Token::Root,  [] (const Group &g) -> Group {
			Group ret;
			for (auto &c: g) {
				ret.push_back(generateCpp(c));
			}
			return ret;
		}},

		{Token::Assignment,  [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw VerificationError(g.token, "wrong number of children in assignment");
			}

			auto op = g[1];
			op.token.leadingSpace = " ";

			return Group({generateCpp(g.front()), op, generateCpp(g.back())});
		}},

		{Token::SetStatement,  [] (const Group &g) -> Group {
			if (g.size() != 4) {
				throw VerificationError(g.token, "set statement has wrong size");
			}
			return Group({generateCpp(g[1]), g[2], generateCpp(g.back())});
		}},

		{Token::ComparisonOperation,  [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw VerificationError(g.token, "wrong number of children in comparison");
			}
			auto op = g[1];
			switch (op.type()) {
			case Token::NotEqual:
			case Token::IsNot:
				op = Token("!= ", op.location());
				break;
			case Token::Equal:
			case Token::Is:
				op = Token("== ", op.location());
				break;
			case Token::Mod:
				op = Token("% ", op.location());
				break;
			case Token::And:
			case Token::AndAlso:
				op = Token("&& ", op.location());
				break;
			case Token::Or:
			case Token::OrElse:
				op = Token("|| ", op.location());
				break;
			default:
				break;
			}

			op.token.leadingSpace = " ";

			return Group({generateCpp(g.front()), op, generateCpp(g.back())});
		}},


		{Token::StringConcatenation, [] (const Group &g) -> Group {
			ExpectSize(g, 3);
			return Group({
				Token("std::to_string(", g.location()),
				generateCpp(g.front()),
				Token(") + std::to_string(", g.location()),
				generateCpp(g.back()),
				Token(")", g.location()),
			});
		}},

		{Token::Literal,  [] (const Group &g) -> Group {
			return g;
		}},

		{Token::Exponentiation,  [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw VerificationError(g.token, "wrong number of children in exp");
			}
			return Group({
				Token("::pow(", g.location()),
				generateCpp(g.front()),
				Token(", ", g.location()),
				generateCpp(g.back()),
				Token(")", g.location()),
			});
		}},

		{Token::IntegerDivision,  [] (const Group &g) -> Group {
			throw VerificationError(g.token, "implement integer division");
		}},

		{Token::ExclusiveDisjunction,  [] (const Group &g) -> Group {
			//For xor look here https://stackoverflow.com/questions/1596668/logical-xor-operator-in-c
			if (g.size() != 3) {
				throw VerificationError(g.token, "wrong number of children in xor");
			}
			return Group({
				Token("!(", g.location()),
				generateCpp(g.front()),
				Token(") != !(", g[1].location()),
				generateCpp(g.back()),
				Token(")", g.back().location())
			});
		}},

		{Token::UnaryIdentityOrNegation,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong number of children");
			}
			return Group ({g.front(), generateCpp(g.back())});
		}},

		{Token::LogicalNegation,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong number of children");
			}
			return Group ({Token("!", g.location()), generateCpp(g.back())});
		}},

		{Token::True,  [] (const Group &g) -> Group {
			return Token("true", g.location());
		}},

		{Token::False,  [] (const Group &g) -> Group {
			return Token("false", g.location());
		}},

		{Token::Else,  [] (const Group &g) -> Group {
			return Token("}\n" + getIndent() + "else {", g.location());
		}},

		{Token::ElseIfStatement,  [] (const Group &g) -> Group {
			return Group({Token("}\n" + getIndent() + "else if (", g.location()), generateCpp(g[1]), Token(") {", g.location())});
		}},

		{Token::Parenthesis,  [] (const Group &g) -> Group {
			if (g.size() != 1) {
				throw VerificationError(g.token, "parenthesis not parsed properly");
			}
			return Group({Token("(", g.location()), generateCpp(g.front()), Token(")", g.endToken.location())});
		}},

		{Token::PropertyAccessor,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong size on property accessor");
			}

			return Group({Token("_with->", g.location()), generateCpp(g.back())});
		}},

		{Token::FunctionCallOrPropertyAccessor,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong size in funuction call or property accessor");
			}
			if (g.back().type() == Token::PropertyAccessor) {
				auto &propertyAccessor = g.back();
				ExpectSize(g, 2);
				return Group({generateCpp(g.front()), Token("->", g.location()), generateCpp(propertyAccessor.back())});
			}
			else if (g.back().type() == Token::Parenthesis) {
				return Group({generateCpp(g.front()), generateCpp(g.back())});
			}
			throw VerificationError(g.token, "statement is missing property accessor or parenthesis");
		}},

		{Token::MethodCall,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong size in method call");
			}
			return Group({generateCpp(g.front()), Token("(", g.location()), generateCpp(g.back()), Token(")", g.location())});
		}},


		{Token::LogicalNegation,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong number of children in logical negation");
			}
			return Group({Token("!", g.location()), generateCpp(g[1])});
		}},

		{Token::InlineIfStatement,  [] (const Group &g) -> Group {
			if (g.size() != 4) {
				throw VerificationError(g.token, "wrong number of children in inline if statement");
			}

			return Group({Token("if (", g.location()), generateCpp(g[1]), Token(") ", g.location()), generateCpp(g[3])});
//			return Group();
		}},

		{Token::Word,  [] (const Group &g) -> Group {
			if (!g.empty()) {
				throw VerificationError(g.token, "Word has children");
			}
			return Token(g.token.wordSpelling(), g.location());
		}},

		{Token::ExitStatement,  [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			auto type = g.back().type();
			if (type == Token::Sub) {
				return Token("return", g.location());
			}
			else if (type == Token::Function) {
				return Token("return " + currentFunction.name, g.location());
			}
			else if (type == Token::Do) {
				return Token("break", g.location());
			}
			throw GenerateError(g, "could not create exit statement for this");
		}},


		{Token::Me,  [] (const Group &g) -> Group {
			return Token("shared_from_this()", g.location());
		}},

		{Token::Line,  [] (const Group &g) -> Group {
			Group ret;
//			cout << "line" << endl;
			if (!g.empty()) {
				auto frontType = g.front().type();
				if (frontType == Token::ElseIfStatement || frontType == Token::Else) {
					--depth;
					auto ret = Group({Token(getIndent(), g.location()), generateCpp(g.front()), Token("\n", g.location())});
					++depth;
					return ret;
				}
				else {
					auto ret = Group({Token(getIndent(), g.location()), generateCpp(g.front()), Token(";\n", g.location())});

					if (ret.type() == Token::Remove) {
						return Group(Token::Remove);
					}
					else {
						return ret;
					}
				}
			}
			return ret;
		}},

		{Token::Block,  [] (const Group &g) -> Group {
			Group ret;

			auto blockType = g.front().front().type();

			string indent(depth * tabSize, ' ');

			if (blockType == Token::AccessSpecifier) {
				if (auto name = g.front().getByType(Token::Word)) {
					cout << "function with name " << name->spelling() << endl;
					string typeName = "void";
					if (auto asClause = g.front().getByType(Token::AsClause)) {
						auto typetoken = asClause->back();
						typeName = generateTypeString(typetoken.token);
						cout << "and with type " << typeName << endl;
					}
					ret.push_back(Token(indent + typeName + " " + name->spelling() +  "(", g.location()));
					if (auto p = g.front().getByType(Token::Parenthesis)) {
						if (p->size() > 0) {
							ret.push_back(generateCpp(p->front()));
						}
					}
					ret.push_back(Token(") {\n", g.location()));
					ret.push_back(Token(getIndent(depth + 1) + typeName + " " + name->spelling() + ";\n", g.location()));
				}
				else {
					cout << "could not find a name for the block" << endl;
				}
			}
			else if (blockType == Token::IfStatement) {
				auto &blockStart = g.front();
				ret.push_back(Token(getIndent(depth) + "if (", g.location()));
				ret.push_back(generateCpp(blockStart.front()[1]));
				ret.push_back(Token(") {\n", g.location()));
			}
			else if (blockType == Token::WithStatement) {
				ret.push_back(Token(getIndent(depth), g.location()));

				auto with = g.front().front();
//				ret.push_back(generateCpp(g.front().front()));

				ExpectSize(with, 2);
				ret.children.push_back(Group({Token("{\n" + getIndent(depth + 1) + "auto &_with = ", with.location()), generateCpp(with.back())}));

				ret.push_back(Token("\n", g.location()));
			}

			++depth;
			for (auto it = g.begin() + 1; it != (g.end() - 1); ++it) {
				ret.push_back(generateCpp(*it));
			}
			--depth;


			ret.push_back(Token(indent + "}\n", g.back().location()));

			return ret;
		}},

		{Token::AccessSpecifier, [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw GenerateError(g, "Not wrong amount of groups in statement");
			}

			auto type = g[1].type();

//			cout << "-- type for list or something = " << g.front().typeString() << endl;

			if (type == Token::CommaList) {
				auto &cl = g[1];
				if (cl.size() != 3) {
					throw GenerateError(g, "wrong sie of list element");
				}

				auto list = cl.flattenCommaList();

				Group ret;

				for (int i = 0; i < list.size(); i += 2) {
					auto d = list[i];
					if (i > 0) {
						ret.push_back(Token("; ", d.location()));
					}
					ret.push_back(generateCpp(d));
				}

				return ret;

			}
			else if(type == Token::AsClause || type == Token::TypeCharacterClause) {
				auto ret = generateCpp(g[1]);
				return ret;
			}
			else if(type == Token::TypeStatement) {
#warning "this should be under the block token"
				return generateCpp(g[1]);
			}
			else if(type == Token::ConstStatement) {
				return generateCpp(g[1]);
			}
			else if (type == Token::DeclareStatement) {
				return generateCpp(g[1]);
			}

			throw VerificationError(g.token, "Could not do anything with expression");
		}},


		{Token::CommaList, [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw GenerateError(g, "wrong sie of list element");
			}

			auto list = g.flattenCommaList();

			Group ret;

			for (int i = 0; i < list.size(); i += 2) {
				auto d = list[i];
				if (i > 0) {
					ret.push_back(Token(", ", d.location()));
				}
				ret.push_back(generateCpp(d));
			}

			return ret;
		}},


		{Token::TypeStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			return Group({Token("struct ", g.location()), generateCpp(g.back()), Token(" {", g.location())});
		}},

		{Token::TypeStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 8);
			cerr << "declare statement is not implemented" << endl;
			return Group();
		}},

		{Token::ConstStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			if (auto assignment = g.getByType(Token::Assignment)) {
				ExpectSize(*assignment, 3);
				return Group({
					Token("const auto ", g.location()),
							assignment->front(),
							assignment->at(1),
							generateCpp(assignment->back())});
			}
			throw VerificationError(g.token, "const expression dit not contain assignment");
		}},

//		{Token::SubStatement, [] (const Group &g) -> Group {
//			Group ret;
//
//			return ret;
//		}},

		{Token::RefTypeStatement, [] (const Group &g) -> Group {
			return generateCpp(g.back());
#warning "implement byref";
		}},


		{Token::OptionalStatement, [] (const Group &g) -> Group {
			auto ret = generateCpp(g.back());

			ret.push_back(Token(" = 0", g.location()));
			return ret;
		}},

		{Token::NewStatement, [] (const Group &g) -> Group {
			return Group({Token("std::make_shared<" + g[1].token.wordSpelling() + ">()", g.location())});
		}},

		{Token::TypeCharacterClause, [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw GenerateError(g, "Wrong amount of arguments in type character clause");
			}

			Group ret = g.token;

			string type = "int";

			switch (g[1].type()) {
			case Token::Percentage:
				type = "int";
				break;
			case Token::Et:
				type = "long";
				break;
			case Token::At:
				type = "long double";
				break;
			case Token::Exclamation:
				type = "float";
				break;
			case Token::Hash:
				type = "double";
				break;
			case Token::Dollar:
				type = "std::string";
				break;
			default:
				break;
			}

			ret.push_back(Token(type + " " + g[0].token.wordSpelling(), g.location()));

			return ret;
		}},

		{Token::OptionStatement, [] (const Group &g) -> Group {
			return Group(Token::Remove);
		}},

		{Token::AsClause, [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw GenerateError(g, "Wrong amount of arguments in type character clause");
			}

			Group ret = g.token;

			string type = generateTypeString(g[2].token);

			ret.push_back(Token(type + " " + g[0].token.wordSpelling(), g.location()));

			return ret;
		}},
};

static class InitOutputClass {
public:
	InitOutputClass() {
		genMap[Token::DimStatement] = genMap.at(Token::AccessSpecifier);
		genMap[Token::Numeric] = genMap.at(Token::Word);
		genMap[Token::AdditionOrSubtraction] = genMap.at(Token::ComparisonOperation);
		genMap[Token::MultiplicationOrDivision] = genMap.at(Token::AdditionOrSubtraction);
		genMap[Token::ModulusOperation] = genMap.at(Token::AdditionOrSubtraction);
		genMap[Token::Conjunction] = genMap.at(Token::AdditionOrSubtraction);
		genMap[Token::InclusiveDisjunction] = genMap.at(Token::AdditionOrSubtraction);
	}
} initClass;


void loadTypeDeclarations(string rootFolder) {

}


Group generateCpp(const Group &g) {
//	cout << "trying to generate for " << g.location() << g.typeString() << endl;

	if (g.type() == Token::Heading) {
		return Group();
	}
	try {
//		cout << "looking for " << g.typeString() << endl;
		auto f = genMap.at(g.type());
		return f(g);
	}
	catch (out_of_range &e) {
		cout << "could not find expression for " << g.location() << g.typeString() << endl;
		return Group();
	}
	catch (runtime_error &e) {
		cout << "error when trying to generate from" << endl;
		g.printRecursive(cout, 0);
	}
	return Group();
}


} // namespace vbconv


using namespace vbconv;

int main(int argc, char **argv) {
//	string filename = "originals/SD/Ship.cls";
	string filename = "originals/SD/MainMod.bas";
	loadTypeInformation(getDirectory(filename));
	File f(filename);

	cout << "// Lasersköld vb6conv cpp-generator" << endl;

	auto output = generateCpp(f.tokens);

	auto ending = getEnding(filename);

	auto unitName = getFileName(filename);

	if (ending == "cls") {
		output.children.insert(output.begin(), Group({Token("class " + unitName + " {\n", output.location())}));
		output.push_back(Token("};", output.location()));
	}

	cout << output.spelling() << endl;
}
