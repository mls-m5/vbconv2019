/*
 * typelibrary.h
 *
 *  Created on: 22 apr. 2019
 *      Author: mattias
 */


#include "typelibrary.h"
#include "token.h"
#include "gencpp.h"
#include <string>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <sstream>

#include "common.h"

using namespace std;

namespace vbconv {


static std::vector<TypeDeclaration> declaredTypes = {};


vector<string> listFiles(const string &directory) {
	vector <string> ret;
	DIR *dir = opendir(directory.c_str());

	if (dir) {
		while (dirent *ent = readdir(dir)) {
			string name = ent->d_name;
			if (name != "." && name != "..") {
				ret.emplace_back(directory + "/" + name);
			}
		}
		closedir(dir);
	}
	else {
		cerr << "could not open directory " << directory << endl;
	}
	return ret;
}

bool isDirectory(const string &path) {
	struct stat fileStat;
	int error = stat(path.c_str(), &fileStat);
	if (error) {
		cerr << "file or directory " << path << " does not exist" << endl;
	}
	return fileStat.st_mode & S_IFDIR;
}

vector<string> findAllFilesRecursive(const string &directory) {
	auto list = listFiles(directory);
	vector<string> ret;

	for (auto &f: list) {
		if (isDirectory(f)) {
			auto recurse = findAllFilesRecursive(f);
			ret.reserve(ret.size() + recurse.size());
			move(recurse.begin(), recurse.end(), back_inserter(ret));
		}
		else {
			ret.push_back(f);
		}
	}

	return ret;
}

void fastParseFileForDeclarations(string filename) {
	fstream file(filename);

	string word;
	string prevWord;
	auto activeUnitName = getCurrentUnitName();
	auto unitName = getUnitName(filename);

	// Private types and enums is included in header file
	bool includePrivate = unitName == activeUnitName;
//	bool includePrivate = true;

	while (file >> word) {
		transform(word.begin(), word.end(), word.begin(), ::tolower);

		if ((word == "type" || word == "enum")  && prevWord != "end") {

			ScopeType type;
			if (word == "type") {
				type = ScopeType::Type;
			}
			else {
				type = ScopeType::Enum;
			}
			file >> word;

			if (type == ScopeType::Type && !includePrivate && prevWord != "public") {
				vout << "skipping private type " << word << endl;
				continue;
			}
			else if (type == ScopeType::Enum && !includePrivate && prevWord == "private") {
				//Enums are public by default
				vout << "skipping private enum " << word << endl;
				continue;
			}
			vout << "in " << filename << " " << " found definition of " << word << endl;
			declaredTypes.emplace_back(word, type, filename);
			prevWord.clear();


			if (type == ScopeType::Enum) {
				string line;
				getline(file, line); //Clears the last line
				while (getline(file, line)) {
					stringstream ss(line);
					ss >> word;

					if (toLower(word) == "end") {
						dout << "end of enum" << endl;
						break;
					}

					vout << "added reference to enum value (const) " << word << endl;
					declaredTypes.emplace_back(word, ScopeType::Const, filename);
				}
			}
		}
		else if ((word == "sub" || word == "function") && (prevWord != "end" && prevWord != "exit")) {
			file >> word;
			if (!includePrivate && prevWord != "public") {
				vout << "skipping private function " << word << endl;
				continue;
			}
			vout << "in " << filename << " " << " found definition of function " << word << endl;
			declaredTypes.emplace_back(word, ScopeType::Function, filename);
			prevWord.clear();
		}
		else if(prevWord == "public") {
			auto type = ScopeType::Variable;
			if (word == "const") {
				file >> word;
				vout << "const: ";
				type = ScopeType::Const;
			}

			else {
				vout << "in " << filename << " " << " found definition of variable " << word << endl;
				declaredTypes.emplace_back(word, type, filename);
			}
			prevWord.clear();
		}
		swap(word, prevWord);
	}
}

void loadTypeInformation(string fileOrRootFolder) {
	vector<string> files;
	if (isDirectory(fileOrRootFolder)) {
		files = findAllFilesRecursive(fileOrRootFolder);
	}
	else {
		files = {fileOrRootFolder};
	}

	for (auto &f: files) {
		auto ending = toLower(getEnding(f));
		auto strippedName = getFileName(f);
		if (!ending.empty() && strippedName.size() > ending.size() + 1) {
			strippedName = string(strippedName.begin(), strippedName.end() - ending.size() - 1);
		}
		vout << "checking file with ending " << ending << " --> " << getFileName(f) << endl;
		if (ending == "cls" || ending == "frm") {
			vout << "found class file for class " << strippedName << endl;
			declaredTypes.emplace_back(strippedName, ScopeType::Class, f);
			fastParseFileForDeclarations(f);
		}
		else if (ending == "bas") {
			vout << "found module " << strippedName << endl;
			declaredTypes.emplace_back(strippedName, ScopeType::Module, f);
			fastParseFileForDeclarations(f);
		}
	}
}


std::string getUnitForSymbol(const std::string &symbol) {
	auto lcase = symbol;
	transform(lcase.begin(), lcase.end(), lcase.begin(), ::tolower);
	for (auto &type: declaredTypes) {
		if (type.name == lcase) {
			return type.sourceUnit;
		}
	}
	vout << "Undefined reference to " << symbol << endl;
	return "";
}



TypeDeclaration::TypeDeclaration(std::string n, ScopeType type, std::string source) :
		casedName(stripNonAlphaNumeric(n)), name(casedName.size(), ' '), type(type), sourceFile(source) {

	std::transform(casedName.begin(), casedName.end(), name.begin(), ::tolower);

	sourceUnit = getUnitName(source);

	vout << "Added reference to " << casedName << " in unit " << sourceUnit << " type " << (int)type << endl;
}


TypeDeclaration* findTypeDeclaration(const std::string &typeName) {
	string lowerCaseTypeName = toLower(typeName);
	for (auto& t : declaredTypes) {
		if (t == lowerCaseTypeName) {
			return &t;
		}
	}

	return nullptr;
}

Group generateTypeGroup(const Group &vbtype) {
	Token::Location location = vbtype.location();

	// Helper function to create a token fast
	auto ct = [location](string str, Token::Type type = Token::None) {
		return Token(str, location, type);
	};

	switch (vbtype.type()) {
	case Token::Integer:
		return ct("int");
		break;
	case Token::Long:
		return ct("long");
		break;
	case Token::At:
		return ct("long double");
		break;
	case Token::Single:
		return ct("float");
		break;
	case Token::Double:
		return ct("double");
		break;
	case Token::Boolean:
		return ct("bool");
		break;
	case Token::Currency:
		return ct("Currency");
		break;
	case Token::String:
		return ct("VBString");
		break;

	default:
		break;
	}

	auto typeToken = vbtype.token;
	if (auto typeDecl = findTypeDeclaration(typeToken)) {
		if (typeDecl->type == ScopeType::Class) {
			vout << "found class " << typeToken.wordSpelling() << " creating shared_ptr" << endl;

			addReferenceToClass(typeDecl->casedName);
			return Group({ct("std::shared_ptr<"), ct(typeDecl->casedName, Token::CReference), ct(">")}, Token::CPointerType);
		}
		else {
			addReferenceToType(typeDecl->casedName);
			return ct(typeDecl->casedName, Token::CReference);
		}
	}
	else {
		vout << "could not find type with name " << typeToken.wordSpelling() << endl;
		return ct(typeToken.wordSpelling(), Token::CReference);
	}
}

}  // namespace vbconv
