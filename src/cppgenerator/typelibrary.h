
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


struct TypeDeclaration {
	enum Kind {
		None,
		Type,
		Enum,
		Class,
		Module,
		Array,
		Function,
	};

	std::string casedName;
	std::string name;
	Kind type = None;

	std::vector<TypeDeclaration> members;

	TypeDeclaration(std::string n, Kind type = Kind::Type);
	TypeDeclaration() {}

	bool operator == (const std::string &n) {
		return n == name;
	}
};


TypeDeclaration* findTypeDeclaration(std::string lowerCaseTypeName);

}  // namespace vbconv



