
/*
 * typelibrary.cpp
 *
 *  Created on: 22 apr. 2019
 *      Author: mattias
 */


#include <string>
#include <vector>
#include "token.h"

namespace vbconv {

//Load type information from file or for all files in folder
void loadTypeInformation(std::string fileOrRootFolder);

Group generateTypeGroup(const class Group &vbtype);

std::string getUnitForSymbol(const std::string &symbol);

enum class ScopeType {
	None,
	Type,
	Enum,
	Class,
	Module,
	Array,
	Variable,
	Const,
	Function,
	FunctionArguments,
};

struct TypeDeclaration {
	std::string casedName;
	std::string name;
	ScopeType type = ScopeType::None;
	std::string sourceFile;
	std::string sourceUnit;

	std::vector<TypeDeclaration> members;

	TypeDeclaration(std::string n, ScopeType type, std::string sourceFile);
	TypeDeclaration() {}

	bool operator == (const std::string &n) {
		return n == name;
	}
};


TypeDeclaration* findTypeDeclaration(const std::string &typeName);

}  // namespace vbconv



