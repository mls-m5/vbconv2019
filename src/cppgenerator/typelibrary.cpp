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
#include <sys/stat.h>
#include <algorithm>

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

string getEnding(const string &filename) {
	for (int i = filename.size() - 1; i >= 0; --i) {
		if (filename[i] == '/' || filename [i] == '\\') {
			return "";
		}
		else if (filename[i] == '.') {
			return string(filename.begin() + (i + 1), filename.end());
		}
	}
	return "";
}

string getFileName(const string &path) {
	for (int i = path.size() - 1; i >= 0; --i) {
		if (path[i] == '/' || path [i] == '\\') {
			return string(path.begin() + i + 1, path.end());
		}
	}
	return path;
}

string getDirectory(const string &path) {
	for (int i = path.size() - 1; i >= 0; --i) {
		if (path[i] == '/' || path [i] == '\\') {
			return string(path.begin(), path.begin() + i);
		}
	}
	return path;
}

string toLower(string str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

void loadTypeInformation(string rootFolder) {
	auto files = findAllFilesRecursive(rootFolder);

	for (auto &f: files) {
		auto ending = toLower(getEnding(f));
		auto strippedName = getFileName(f);
		if (!ending.empty() && strippedName.size() > ending.size() + 1) {
			strippedName = string(strippedName.begin(), strippedName.end() - ending.size() - 1);
		}
		vout << "loaded file with ending " << ending << " --> " << getFileName(f) << endl;
		if (ending == "cls" || ending == "frm") {
			vout << "found class file for class " << strippedName << endl;
			declaredTypes.emplace_back(strippedName, TypeDeclaration::Class);
		}
		else if (ending == "bas") {
			vout << "found module " << strippedName << endl;
			declaredTypes.emplace_back(strippedName, TypeDeclaration::Module);
		}
	}
}

TypeDeclaration::TypeDeclaration(std::string n, Kind type) :
		casedName(n), name(n.size(), ' '), type(type) {
	std::transform(casedName.begin(), casedName.end(), name.begin(), ::tolower);
}

TypeDeclaration* findTypeDeclaration(std::string lowerCaseTypeName) {
	for (auto& t : declaredTypes) {
		if (t == lowerCaseTypeName) {
			return &t;
		}
	}
	return nullptr;
}

string generateTypeString(const Group &vbtype) {
	auto type = vbtype.type();

	switch (type) {
	case Token::Integer:
		return "int";
		break;
	case Token::Long:
		return "long";
		break;
	case Token::At:
		return "long double";
		break;
	case Token::Single:
		return "float";
		break;
	case Token::Double:
		return "double";
		break;
	case Token::Boolean:
		return "bool";
		break;
	case Token::String:
		return "std::string";
		break;

	default:
		break;
	}

	auto typeToken = vbtype.token;
	if (auto typeDecl = findTypeDeclaration(typeToken)) {
		if (typeDecl->type == TypeDeclaration::Class) {
			vout << "found class " << typeToken.wordSpelling() << " creating shared_ptr" << endl;

			return "std::shared_ptr<" + typeDecl->casedName + ">";

		}
		else {
			return typeDecl->casedName;
		}
	}
	else {
		vout << "could not find type with name " << typeToken.wordSpelling() << endl;
		return typeToken.wordSpelling();
	}
}

}  // namespace vbconv
