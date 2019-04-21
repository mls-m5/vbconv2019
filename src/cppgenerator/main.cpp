/*
 * main.cpp
 *
 *  Created on: 19 apr. 2019
 *      Author: mattias
 */

#include <file.h>
#include <iostream>
#include <map>
#include <functional>
#include <sstream>

using namespace std;
using namespace vbconv;

Group generateCpp(const Group &g);

int depth = 1;
int tabSize = 4;

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
				op = Token("!= ", op.location());
				break;
			case Token::Equal:
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
			}

			op.token.leadingSpace = " ";

			return Group({generateCpp(g.front()), op, generateCpp(g.back())});
		}},


		{Token::Exponentiation,  [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw VerificationError(g.token, "wrong number of children in exp");
			}
			return Group({
				Token("pow(", g.location()),
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

			return Group({g.front(), generateCpp(g.back())});
		}},

		{Token::FunctionCallOrPropertyAccessor,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw VerificationError(g.token, "wrong size in funuction call or property accessor");
			}
			if (g.back().type() == Token::PropertyAccessor) {
				return Group({generateCpp(g.front()), generateCpp(g.back())});
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
		}},

		{Token::Word,  [] (const Group &g) -> Group {
			if (!g.empty()) {
				throw VerificationError(g.token, "Word has children");
			}
			return Token(g.token.wordSpelling(), g.location());
		}},

		{Token::Line,  [] (const Group &g) -> Group {
			Group ret;
//			cout << "line" << endl;
			if (!g.empty()) {
				return Group({Token(string(depth * tabSize, ' '), g.location()), generateCpp(g.front()), Token(";\n", g.location())});
			}
			return ret;
		}},

		{Token::Block,  [] (const Group &g) -> Group {
			Group ret;
//			cout << "block " << g.front().front().typeString() << endl;

			auto blockType = g.front().front().type();

			string indent(depth * tabSize, ' ');

			if (blockType == Token::AccessSpecifier) {
//				cout << "this is probably a sub or function statement" << endl;
				if (auto name = g.front().getByType(Token::Word)) {
					cout << "function with name " << name->spelling() << endl;
					ret.push_back(Token(indent + "void " + name->spelling() +  "(", g.location()));
					if (auto p = g.front().getByType(Token::Parenthesis)) {
						if (p->size() > 0) {
							ret.push_back(generateCpp(p->front()));
						}
					}
					ret.push_back(Token(") {\n", g.location()));


				}
				else {
					cout << "could not find a name for the block" << endl;
				}

			}
			else if (blockType == Token::IfStatement) {
				auto &blockStart = g.front();
				ret.push_back(Token(indent + "if (", g.location()));
				ret.push_back(generateCpp(blockStart.front()[1]));
				ret.push_back(Token(") {\n", g.location()));
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

		{Token::SubStatement, [] (const Group &g) -> Group {
			Group ret;

			return ret;
		}},

		{Token::RefTypeStatement, [] (const Group &g) -> Group {
			return generateCpp(g.back());
#warning "implement byref";
		}},


		{Token::OptionalStatement, [] (const Group &g) -> Group {
			auto ret = generateCpp(g.back());

			ret.push_back(Token(" = 0", g.location()));
			return ret;
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

		{Token::AsClause, [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw GenerateError(g, "Wrong amount of arguments in type character clause");
			}

			Group ret = g.token;

			string type = "int";

			switch (g[2].type()) {
			case Token::Integer:
				type = "int";
				break;
			case Token::Long:
				type = "long";
				break;
			case Token::At:
				type = "long double";
				break;
			case Token::Single:
				type = "float";
				break;
			case Token::Double:
				type = "double";
				break;
			case Token::String:
				type = "std::string";
				break;
			default:
				type = g[2].token.wordSpelling();
				cout << "undefined type" << g[2].token.cased << endl;
				break;
			}

			ret.push_back(Token(type + " " + g[0].token.wordSpelling(), g.location()));

			return ret;
		}},
};

static class InitClass {
public:
	InitClass() {
		genMap[Token::DimStatement] = genMap.at(Token::AccessSpecifier);
		genMap[Token::Numeric] = genMap.at(Token::Word);
		genMap[Token::AdditionOrSubtraction] = genMap.at(Token::ComparisonOperation);
		genMap[Token::MultiplicationOrDivision] = genMap.at(Token::AdditionOrSubtraction);
		genMap[Token::ModulusOperation] = genMap.at(Token::AdditionOrSubtraction);
		genMap[Token::Conjunction] = genMap.at(Token::AdditionOrSubtraction);
		genMap[Token::InclusiveDisjunction] = genMap.at(Token::AdditionOrSubtraction);
	}
} initClass;

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
	return Group();
}


int main(int argc, char **argv) {
	File f("originals/SD/Ship.cls");

	cout << "// Lasersköld vb6conv cpp-generator" << endl;

	auto output = generateCpp(f.tokens);

	cout << output.spelling() << endl;
}
