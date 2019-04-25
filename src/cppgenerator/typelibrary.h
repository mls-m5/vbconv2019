
/*
 * typelibrary.cpp
 *
 *  Created on: 22 apr. 2019
 *      Author: mattias
 */


#include <string>
#include <vector>

namespace vbconv {


void loadTypeInformation(std::string rootFolder);
std::string getDirectory(const std::string &);
std::string getEnding(const std::string &filename);

//Returns the filename without path name
std::string getFileName(const std::string &path);
std::string generateTypeString(const class Group &vbtype);

enum class ScopeType {
	None,
	Type,
	Enum,
	Class,
	Module,
	Array,
	Function,
	FunctionArguments,
};

struct TypeDeclaration {
	std::string casedName;
	std::string name;
	ScopeType type = ScopeType::None;

	std::vector<TypeDeclaration> members;

	TypeDeclaration(std::string n, ScopeType type = ScopeType::Type);
	TypeDeclaration() {}

	bool operator == (const std::string &n) {
		return n == name;
	}
};


TypeDeclaration* findTypeDeclaration(std::string lowerCaseTypeName);

}  // namespace vbconv



