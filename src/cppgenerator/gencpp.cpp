/*
 * gencpp.cpp
 *
 * This file consists of four parts
 * 1. Settings and warnings - stuff you need to setup before you start
 * 2. The functions for creating code
 * 3. A initialization function for duplicate functions
 * 4. The main organizing function to be used
 *  Created on: 23 apr. 2019
 *      Author: mattias
 */




#include "gencpp.h"
#include "typelibrary.h"
#include <file.h>
#include <iostream>
#include <map>
#include <functional>
#include <sstream>
#include <algorithm>
#include <set>

using namespace std;

namespace vbconv {

bool verboseOutput = false;

Group generateCpp(const Group &g);

static int depth = 0;
static int tabSize = 4;

struct {
	string name;
	TypeDeclaration type;
} currentFunction;

string currentLineEnding = ";";

struct {
	vector <Token> members;
	string name = "";
} currentType;


static struct {
	bool headerMode = true;
	string filename;
	string unitName;
	string ending;
	Token::Type unitType = Token::Class; //Can be Module or Class

	Token::Type currentAccessLevel = Token::Private;

	set<string> classReferences;

	TypeDeclaration::Kind currentScopeKind = TypeDeclaration::None;

	void clear() {
		classReferences.clear();
		currentAccessLevel = Token::Private;
		ending.clear();
		unitName.clear();
		filename.clear();
		currentScopeKind = TypeDeclaration::None;
	}

} settings;

void setHeaderMode(bool mode) {
	settings.headerMode = mode;
}

void setFilename(string filename) {
	settings.filename = filename;
	auto file = getFileName(filename);
	settings.ending = getEnding(filename);
	settings.unitName = string(file.begin(), file.end() - settings.ending.size() - 1);

	if (settings.ending == "bas") {
		settings.unitType = Token::Module;
	}
}

void addReferenceToClass(std::string className) {
	settings.classReferences.insert(className);
}

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
		ss << g.token.location() << ":" << message << "\n" << g.spelling() << "\n";
		g.printRecursive(ss);
		this->message = ss.str();
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

void ExpectSize(const Group &g, std::initializer_list<int> sizes) {
	auto found = false;
	for (auto size: sizes) {
		if (size == g.size()) {
			found = true;
		}
	}
	if (!found) {
		throw GenerateError(g, "wrong size in expression " + g.typeString());
	}
}

typedef Group mapFunc_t (const Group &);

map<Token::Type, mapFunc_t*> genMap = {
		{Token::Root,  [] (const Group &g) -> Group {
			Group ret;
			auto lcaseName = settings.unitName;
			transform(lcaseName.begin(), lcaseName.end(), lcaseName.begin(), ::tolower);

			string headerString =  "// Generated with Lasersköld vb6conv cpp-generator\n\n";


			if (settings.headerMode) {
				depth = 1;
				headerString += "#pragma once\n\n";
			}
			else {
				depth = 0;
			}


			for (auto &c: g) {
				auto l = generateCpp(c);
				if (l != Token::Remove) {
					ret.push_back(move(l));
				}
			}

			for (auto &c: settings.classReferences) {
				headerString += "class " + c + ";\n";
			}
			headerString += "\n";

			if (settings.ending == "cls") {
				if (settings.headerMode) {
					ret.children.insert(ret.begin(), Group({Token("class " + settings.unitName + ": public std::enable_shared_from_this<" + settings.unitName + "> {\npublic:\n", ret.location())}));
					ret.push_back(Token("};\n", ret.location()));
				}
			}
			else if (settings.unitType == Token::Module) {
//				unitName = string(unitName.begin(), unitName.end() - 4);
				ret.children.insert(ret.begin(), Token("namespace " + settings.unitName + " {\n", ret.location()));
				ret.push_back(Token("} // namespace \nusing namespace " + settings.unitName + ";\n", ret.location()));
			}

			if (settings.headerMode) {
				ret.children.insert(ret.children.begin(), Token(headerString + "#include \"vbheader.h\"\n\n", ret.location()));
			}
			else {
				ret.children.insert(ret.children.begin(), Token(headerString + "#include \"" + lcaseName + ".h\"\n\n", ret.location()));
			}

			return ret;
		}},


		{Token::Assignment,  [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw GenerateError(g, "wrong number of children in assignment");
			}

			auto op = g[1];
			op.token.leadingSpace = " ";

			if (g.front().type() == Token::AttributeStatement) {
				return Group(Token::Remove);
			}
			if (g.front().type() == Token::Word && g.front().token.wordSpelling() == currentFunction.name) {
				return Group({Token("_ret", g.location()), op, generateCpp(g.back())});
			}
			else {
				return Group({generateCpp(g.front()), op, generateCpp(g.back())});
			}
		}},


		{Token::SetStatement,  [] (const Group &g) -> Group {
			if (g.size() != 4) {
				throw GenerateError(g, "set statement has wrong size");
			}
			return Group({generateCpp(g[1]), g[2], generateCpp(g.back())});
		}},


		{Token::ComparisonOperation,  [] (const Group &g) -> Group {
			ExpectSize(g, 3);
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
			auto ret = g.token.strip();
			for (auto i = 0; i < ret.size(); ++i) {
				if (ret[i] == '\\') {
					ret.insert(i, "\\");
					++i;
				}
			}
			return ret;
		}},


		{Token::Exponentiation,  [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw GenerateError(g, "wrong number of children in exp");
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
			ExpectSize(g, 3);
			return Group({
				Token("(int)(", g.location()),
						generateCpp(g.front()),
						Token(" / ", g[1].location()),
						generateCpp(g.back()),
						Token(")", g.back().location()),
			});
		}},


		{Token::ExclusiveDisjunction,  [] (const Group &g) -> Group {
			//For xor look here https://stackoverflow.com/questions/1596668/logical-xor-operator-in-c
			ExpectSize(g, 3);

			return Group({
				Token("!(", g.location()),
				generateCpp(g.front()),
				Token(") != !(", g[1].location()),
				generateCpp(g.back()),
				Token(")", g.back().location())
			});
		}},


		{Token::UnaryIdentityOrNegation,  [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			return Group ({g.front(), generateCpp(g.back())});
		}},


		{Token::LogicalNegation,  [] (const Group &g) -> Group {
			ExpectSize(g, 2);
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
			ExpectSize(g, {0, 1});
			if (g.empty()) {
				return Token("()", g.location());
			}
			else {
				return Group({Token("(", g.location()), generateCpp(g.front()), Token(")", g.endToken.location())});
			}
		}},


		{Token::PropertyAccessor,  [] (const Group &g) -> Group {
			ExpectSize(g, 2);

			return Group({Token("_with->", g.location()), generateCpp(g.back())});
		}},


		{Token::FunctionCallOrPropertyAccessor,  [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			if (g.back().type() == Token::PropertyAccessor) {
				auto &propertyAccessor = g.back();
				ExpectSize(g, 2);
				return Group({generateCpp(g.front()), Token("->", g.location()), generateCpp(propertyAccessor.back())});
			}
			else if (g.back().type() == Token::Parenthesis) {
				//Function call:
				auto functionName = g.front();
				if (functionName != Token::Word ||
						(functionName.type() == Token::Word && functionName.token.wordSpelling() != currentFunction.name)) {
					// Prevent function name to be replaced with _ret in recursion
					functionName = generateCpp(functionName);
				}

				return Group({functionName, generateCpp(g.back())});
			}
			throw GenerateError(g, "statement is missing property accessor or parenthesis");
		}},


		{Token::MethodCall,  [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw GenerateError(g, "wrong size in method call");
			}
			auto methodName = g.front();
			if (methodName.type() == Token::Word && methodName.token.wordSpelling() != currentFunction.name) {
				// This will prevent the functions name to be replaced with _ret in recursion
				methodName = generateCpp(methodName);
			}
//			cout << methodName.strip().spelling() << endl;
			return Group({methodName.strip(), Token("(", g.location()), generateCpp(g.back()), Token(")", g.location())});
		}},


		{Token::LogicalNegation,  [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			return Group({Token("!", g.location()), generateCpp(g[1])});
		}},


		{Token::InlineIfStatement,  [] (const Group &g) -> Group {
			ExpectSize(g, 4);

			return Group({Token("if (", g.location()), generateCpp(g[1]), Token(") ", g.location()), generateCpp(g[3])});
		}},


		{Token::InlineIfElseStatement,  [] (const Group &g) -> Group {
			ExpectSize(g, 6);

			return Group({
				Token("if (", g.location()),
						generateCpp(g[1]),
						Token(") ", g.location()),
						generateCpp(g[3]),
						Token("; else ", g.location()),
						generateCpp(g[5]),
			});
		}},


		{Token::Word,  [] (const Group &g) -> Group {
			if (!g.empty()) {
				throw GenerateError(g, "Word has children");
			}
			if (g.token.wordSpelling() == currentFunction.name) {
				return Token("_ret", g.location());
			}
			else {
				return Token(g.token.wordSpelling(), g.location());
			}
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
			else if (type == Token::Do || type == Token::For) {
				return Token("break", g.location());
			}
			throw GenerateError(g, "could not create exit statement for this");
		}},


		{Token::Me,  [] (const Group &g) -> Group {
			return Token("shared_from_this()", g.location());
		}},


		{Token::Rnd,  [] (const Group &g) -> Group {
			return Token(g.strip().spelling() + "()", g.location());
		}},


		{Token::Line,  [] (const Group &g) -> Group {
			Group ret;
			if (g.size() == 1 && g.front().type() == Token::Word && settings.currentScopeKind == TypeDeclaration::Function) {
				return Token(getIndent() + g.front().token.wordSpelling() + "();\n", g.location());
			}
			else if (!g.empty()) {
				auto frontType = g.front().type();
				if (frontType == Token::ElseIfStatement || frontType == Token::Else) {
					--depth;
					auto ret = Group({Token(getIndent(), g.location()), generateCpp(g.front()), Token("\n", g.location())});
					++depth;
					return ret;
				}
				else if (frontType == Token::CaseStatement) {
					--depth;
					auto ret = Group({Token(getIndent(), g.location()), generateCpp(g.front()), Token("\n", g.location())}, Token::CCaseStatement);
					++depth;
					return ret;
				}
				else {
					auto line =  generateCpp(g.front());

					if (line.type() == Token::Remove) {
						return Group(Token::Remove);
					}
					else {
						return Group({Token(getIndent(), g.location()), line, Token(currentLineEnding + "\n", g.location())});
					}
				}
			}
			return ret;
		}},


		{Token::Block,  [] (const Group &g) -> Group {
			Group ret;

			auto &block = g.front().front();
			auto blockType = block.type();
			auto endBlockString = "}\n"s;
			currentLineEnding = ";";

			ret.push_back(Token(getIndent(), g.location()));

			auto functionStatement = g.front().getByType(Token::SubStatement);
			if (!functionStatement) {
				functionStatement =  g.front().getByType(Token::FunctionStatement);
			}

			if (functionStatement) {
				auto f = generateCpp(*functionStatement);

				if (settings.headerMode) {
					return Group({Token(getIndent(), g.location()), move(f)});
				}

				if (f.type() == Token::CFunctionWithType) {
					endBlockString = "\n" + getIndent(depth + 1) + "return " + "_ret" + /*name->spelling()*/ + ";\n" + getIndent() + "}";
				}
				ret.push_back(move(f));
				settings.currentScopeKind = TypeDeclaration::Function;
			}
			else if (auto typeBlock = block.getByType(Token::TypeStatement)) {
				if (!settings.headerMode) {
					return Group(Token::Remove);
				}

				ret.push_back(generateCpp(*typeBlock));
				endBlockString = "};";
				settings.currentScopeKind = TypeDeclaration::Type;
			}
			else if (auto enumBlock = block.getByType(Token::EnumStatement)) {
				if (!settings.headerMode) {
					return Group(Token::Remove);
				}

				ret.push_back(generateCpp(*enumBlock));
				currentLineEnding = ",";
				endBlockString = "};";
				settings.currentScopeKind = TypeDeclaration::Enum;
			}
			else if (blockType == Token::IfStatement) {
				ret.push_back(Token("if (", g.location()));
				ret.push_back(generateCpp(block[1]));
				ret.push_back(Token(") {\n", g.location()));
			}
			else if (blockType == Token::WithStatement) {
				auto with = g.front().front();

				ExpectSize(with, 2);
				ret.children.push_back(Group({
					Token("{\n" + getIndent(depth + 1) + "auto &_with = ",
							with.location()),
									generateCpp(with.back()), Token(";", g.location())}));

				ret.push_back(Token("\n", g.location()));
			}
			else if (blockType == Token::ForLoop || blockType == Token::SelectCaseStatement || blockType == Token::DoWhileStatement) {
				ret.push_back(generateCpp(block));
			}
			else if (blockType == Token::Do) {
				ret.push_back(Token("do {\n", g.location()));
			}

			// Block content

			++depth;
			bool needsBreak = false;
			auto insertBreak = [&ret, &g] () {
				ret.push_back(Token(getIndent() + "break;\n", g.location()));
			};

			for (auto it = g.begin() + 1; it != (g.end() - 1); ++it) {
				auto line = generateCpp(*it);
				if (line.type() == Token::CCaseStatement) {
					//Inseart "break;" after case statements
					if (needsBreak) {
						insertBreak();
					}
					needsBreak = true;
				}
				ret.push_back(move(line));
			}
			if (needsBreak) {
				insertBreak();
			}
			--depth;

			// End of block

			auto blockEnd = g.back().front();
			if (blockEnd.type() == Token::LoopWhileStatement) {
				ret.push_back(Token(getIndent(), g.back().location()));
				ret.push_back(generateCpp(blockEnd));
				ret.push_back(Token(";\n", g.back().location()));
			}
			else if (!blockEnd.empty() && blockEnd.back() == Token::TypeKeyword) {
				ret.push_back(Token(getIndent(depth + 1) + "friend std::ostream &operator<<(std::ostream &stream, " + currentType.name + " &o) {\n", blockEnd.location()));
				for (auto &m: currentType.members) {
					ret.push_back(Token(getIndent(depth + 2) + "_save(stream, o." + m.strip().spelling() + ");\n", blockEnd.location()));
				}
				ret.push_back(Token(getIndent(depth + 2) + "return stream;\n", blockEnd.location()));
				ret.push_back(Token(getIndent(depth + 1) + "}\n", blockEnd.location()));

				ret.push_back(Token(getIndent(depth + 1) + "friend std::istream &operator>>(std::istream &stream, " + currentType.name + " &o) {\n", blockEnd.location()));
				for (auto &m: currentType.members) {
					ret.push_back(Token(getIndent(depth + 2) + "_load(stream, o." + m.strip().spelling() + ");\n", blockEnd.location()));
				}
				ret.push_back(Token(getIndent(depth + 2) + "return stream;\n", blockEnd.location()));
				ret.push_back(Token(getIndent(depth + 1) + "}\n", blockEnd.location()));

				ret.push_back(Token(getIndent(depth + 1) + currentType.name + " *operator -> () { return this; }\n\n", blockEnd.location()));
				ret.push_back(Token(getIndent(depth) + "};\n", blockEnd.location()));
				currentType.name = "";
				currentType.members.clear();
				settings.currentScopeKind = TypeDeclaration::None;
			}
			else {
				ret.push_back(Token(getIndent() + endBlockString +"\n", g.back().location()));
			}

			if (functionStatement && settings.currentScopeKind == TypeDeclaration::Function) {
				settings.currentScopeKind = TypeDeclaration::None;
				currentFunction.name = "";
			}


			return ret;
		}},


		{Token::LoopWhileStatement,  [] (const Group &g) -> Group {
			ExpectSize(g, 3);
			return Group({
				Token("} while (", g.location()),
						generateCpp(g.back()),
						Token(")", g.location()),
			});
		}},


		{Token::DoWhileStatement,  [] (const Group &g) -> Group {
			ExpectSize(g, 3);
			return Group({
				Token("while (", g.location()),
						generateCpp(g.back()),
						Token(") {\n", g.location()),
			});
		}},



		{Token::ForLoop,  [] (const Group &g) -> Group {
			ExpectSize(g, 4);
			auto variableName = g[1].getByType(Token::Word);
			if (variableName) {
				auto variableSpelling = variableName->strip();
				return Group({
					Token("for (", g.location()),
							generateCpp(g[1]),
							Token("; " + variableSpelling.spelling() + " <= ", g.location()),
							generateCpp(g[3]),
							Token("; ++", g.location()),
							variableSpelling,
							Token(") {\n", g.location()),
				});
			}
			throw GenerateError(g, "could not create foor loop --> could not find variable name");
		}},


		{Token::FunctionStatement,  [] (const Group &g) -> Group {
			if (auto name = g.getByType(Token::Word)) {
				Group ret;
				vout << "function with name " << name->spelling() << endl;
				auto type = "void"s;
				ret.type(Token::CVoidFunction);

				if (g.back().type() == Token::AsClause) {
					auto asClause = g.back();
					auto typetoken = asClause.back();
					type = generateTypeString(typetoken.token);
					vout << "and with type " << type << endl;
					ret.type(Token::CFunctionWithType);

					currentFunction.name = name->token.wordSpelling();
				}

				string namespaceOrClassString = "";

				if (!settings.headerMode && settings.unitType != Token::Module) {
					namespaceOrClassString = settings.unitName;
					if (!namespaceOrClassString.empty()) {
						namespaceOrClassString += "::";
					}
				}

				ret.push_back(Token(type + " " + namespaceOrClassString
						+ name->token.wordSpelling() +  "(", g.location()));

				if (auto p = g.getByType(Token::Parenthesis)) {
					if (p->size() > 0) {
						ret.push_back(generateCpp(p->front()));
					}
				}
				if (settings.headerMode) {
					ret.push_back(Token(");\n", g.location()));
				}
				else {
					ret.push_back(Token(") {\n", g.location()));
					if (type != "void") {
						ret.push_back(Token(getIndent(depth + 1) + type + " _ret" + ";\n", g.location()));
					}
				}

				return ret;
			}
			throw GenerateError(g, "could not create sub or function, no name found");
		}},


		{Token::OpenStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 5);
			auto fileNumberStatement = g.getByType(Token::FileNumberStatement);
			if (!fileNumberStatement) {
				throw GenerateError(g, "no file number statement in open-statement");
			}
			if (fileNumberStatement->back().type() == Token::FileNumberStatement) {
				fileNumberStatement = &fileNumberStatement->back();
			}
			auto filenumber = fileNumberStatement->back().wordSpelling();
			return Group({
				Token ("std::fstream _file" + filenumber + "(", g.location()),
						generateCpp(g[1]),
						Token(", std::ios::binary)", g.location()),
			});
		}},


		{Token::FileNumberStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			auto filenumber = g.back().strip();
			return Group({Token("_file", g.location()), filenumber});
		}},


		{Token::GetStatement, [] (const Group &g) -> Group {
			// This is shared with Token::PutStatement
			ExpectSize(g, 2);
			auto commaList = g.getByType(Token::CommaList);
			if (!commaList) {
				throw GenerateError(g, "Missign commas arguments to get statement");
			}
			auto filenumberGroup = commaList->getByType(Token::FileNumberStatement);
			if (!filenumberGroup) {
				throw GenerateError(g, "could not find file number");
			}
			auto filenumber = filenumberGroup->back();
			auto back = commaList->back();
			string direction = (g.type() == Token::GetStatement)? " >> " : " << ";
			return Group({
				Token("_file" + filenumber.strip().spelling() + direction, g.location()),
						generateCpp(back)
			});
		}},

		{Token::CloseStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);

			auto filenumberGroup = g.back().getByType(Token::FileNumberStatement);
			if (!filenumberGroup) {
				throw GenerateError(g, "could not find file number");
			}
			auto filenumber = filenumberGroup->back();
			return Token("_file" + filenumber.strip().spelling() + ".close()", g.location());
		}},


		{Token::AccessSpecifier, [] (const Group &g) -> Group {
			if (g.size() != 2) {
				throw GenerateError(g, "Not wrong amount of groups in statement");
			}



			bool showExternTag = false;



			settings.currentAccessLevel = g.front().type();
			if (settings.currentAccessLevel == Token::Dim) {
				settings.currentAccessLevel = Token::Private;
			}

			auto type = g[1].type();

//			cout << "accessspecifier set " << g.strip().spelling() << endl;

			// Outside of function and typestatements it is possible that a
			// variable declaration should have "extern" appended to it
			if (currentFunction.name.empty() && currentType.name.empty()) {
				if (settings.unitType == Token::Module) {
//					cout << "is in module " << g.strip().spelling() << endl;
					showExternTag = settings.headerMode;
				}
				else { // Class
//					cout << "is in class " << g.strip().spelling() << endl;
					if (!settings.headerMode) {
						return Group(Token::Remove);
					}
					showExternTag = false;
				}
			}

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

				if (showExternTag) {
					ret.children.emplace(ret.begin(), Token("extern ", g.location()));
				}

				return ret;

			}
			else if(type == Token::AsClause || type == Token::TypeCharacterClause) {
				auto ret = generateCpp(g[1]);

				if (showExternTag) {
					ret.children.emplace(ret.begin(), Token("extern ", g.location()));
				}

				return ret;
			}
			else if(type == Token::ConstStatement) {
				return generateCpp(g[1]);
			}
			else if (type == Token::DeclareStatement) {
				return generateCpp(g[1]);
			}

			throw GenerateError(g, "Could not do anything with expression");
		}},


		{Token::CommaList, [] (const Group &g) -> Group {
			if (g.size() != 3) {
				throw GenerateError(g, "wrong sie of list element");
			}

			auto list = g.flattenCommaList();

			Group ret;

			for (int i = 0; i < list.size(); i += 1) {
				auto d = list[i];
//				if (i > 0) {
//					ret.push_back(Token(", ", d.location()));
//				}
				ret.push_back(generateCpp(d));
			}

			return ret;
		}},


		{Token::Comma, [] (const Group &g) -> Group {
			return Token(", ", g.location());
		}},


		{Token::DoubleComma, [] (const Group &g) -> Group {
			return Token(",0 , ", g.location());
		}},


		{Token::TypeStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			auto name = g.back().token.wordSpelling();
			currentType.name = name;
			currentType.members.clear();
			return Group({Token("struct " + name + " {\n", g.location())});
		}},


		{Token::EnumStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			auto name = g.back().token.wordSpelling();
			return Group({Token("enum " + name + " {\n", g.location())});
		}},


		{Token::DeclareStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 6);
			cerr << "declare statement is not implemented" << endl;
			return Group({Token("// ", g.location()), g.strip()});
		}},


		{Token::ConstStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			auto accessLevel = settings.currentAccessLevel;
			settings.currentAccessLevel = Token::Private;

			// Private consts is sent to the source file public headers is sent to the header file
			bool show = (accessLevel == Token::Private) != settings.headerMode;

			if (currentFunction.name == "" && !show) {
				return Group(Token::Remove);
			}
			if (auto assignment = g.getByType(Token::Assignment)) {
				ExpectSize(*assignment, 3);
				return Group({
					Token("static const auto ", g.location()),
							assignment->front(),
							assignment->at(1),
							generateCpp(assignment->back())});
			}
			else if (auto defaultAsClause = g.getByType(Token::DefaultAsClause)) {
				ExpectSize(g, 2);
				return Group({
					Token("static const ", g.location()),
							generateCpp(*defaultAsClause)});
			}
			throw GenerateError(g, "const expression do not contain assignment");
		}},


		{Token::RefTypeStatement, [] (const Group &g) -> Group {
			return generateCpp(g.back());
#warning "implement byref";
		}},


		{Token::RedimStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			ExpectSize(g.back(), 2);
			return Group({generateCpp(g.back().front()), Token(".resize", g.location()), generateCpp(g.back().back())});
		}},


		{Token::OptionalStatement, [] (const Group &g) -> Group {
			auto ret = generateCpp(g.back());

			if (ret.type() == Token::CDefaultArgument) {
				return ret;
			}
			else {
				ret.push_back(Token(" = 0", g.location()));
				return ret;
			}
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

			ret.push_back(Token(type + " " + g[0].strip().spelling(), g.location()));

			return ret;
		}},


		{Token::SelectCaseStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 3);
			return Group({
				Token("switch (", g.location()),
						generateCpp(g.back()),
						Token(") {\n", g.location()),
			});
		}},


		{Token::CaseStatement, [] (const Group &g) -> Group {
			ExpectSize(g, 2);
			return Group({
				Token("case ", g.location()),
				generateCpp(g.back()),
				Token(":", g.location())
			}, Token::CCaseStatement);
		}},


		{Token::OptionStatement, [] (const Group &g) -> Group {
			return Group(Token::Remove);
		}},


		{Token::DefaultAsClause, [] (const Group &g) -> Group {
			ExpectSize(g, 3);
			return Group({generateCpp(g.front()), g[1], generateCpp(g.back())}, Token::CDefaultArgument);
		}},


		{Token::AsClause, [] (const Group &g) -> Group {
			ExpectSize(g, 3);

			Group ret = g.token;

			string type = generateTypeString(g.back().token);

			bool isExtern = false;

			if (currentFunction.name.empty() && currentType.name.empty()) {
				if (settings.unitType == Token::Module) {
					isExtern = settings.headerMode;
				}
			}

			auto addMember = [] (const Token &t) {
				// Save the name to make it possible to list it when creating type
				if (!currentType.name.empty()) {
					currentType.members.push_back(t);
				}
				return t;
			};

			if (g.front().type() == Token::FunctionCallOrPropertyAccessor) {


				vout << "variable " << g.front().front().token.wordSpelling() << " is array of type " << type << endl;;
				auto name = g.front().front().token.strip();
				auto arguments = generateCpp(g.front().back()).spelling();
				if (arguments == "()") {
					// One important difference between visual basic and cpp
					arguments = "";
				}
				else if (isExtern) {
					arguments = ""; // Arguments should not be appended to variables defined as extern
				}
				ret.push_back(Token(
						"VBArray <" + type + "> "
						+ addMember(name).wordSpelling()
						+ arguments
						, g.location()));


			}
			else if (g.back().type() == Token::NewStatement) {
				auto newTypeName = g.back().back().token.strip();

				settings.classReferences.insert(newTypeName.spelling());

				string arguments = " (new "	+ newTypeName.wordSpelling() + ")";
				if (isExtern) {
					arguments = "";
				}

				ret.push_back(Token("std::shared_ptr<" + addMember(newTypeName).wordSpelling()
						+ "> " + g.front().token.wordSpelling() +
						arguments
						, g.location()));
			}
			else {
				ret.push_back(Token(type + " " + addMember(g.front().token.strip()).wordSpelling(), g.location()));
			}

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
		genMap[Token::SubStatement] = genMap.at(Token::FunctionStatement);
		genMap[Token::PutStatement] = genMap.at(Token::GetStatement);
		genMap[Token::Timer] = genMap.at(Token::Rnd);
		genMap[Token::Randomize] = genMap.at(Token::Rnd);
	}
} initClass;

Group generateCpp(string filename, bool header) {
	settings.clear();

	loadTypeInformation(getDirectory(filename));
	setFilename(filename);
	setHeaderMode(header);
	File f(filename);
	currentFunction.name = "";
	currentType.name = "";
	currentType.members.clear();

	return generateCpp(f.tokens);
}

Group generateCpp(const Group &g) {
	if (g.type() == Token::Heading) {
		return Group();
	}
	try {
		auto f = genMap.at(g.type());
		return f(g);
	}
	catch (out_of_range &e) {
		vout << "could not find expression for " << g.location() << g.typeString() << endl;
		return Group();
	}
	catch (runtime_error &e) {
		vout << "error when trying to generate from" << endl;
		g.printRecursive(cerr, 0);
	}
	return Group();
}


} // namespace vbconv


