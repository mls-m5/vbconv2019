/*
 * vbmake.cpp
 *
 *  Created on: 25 apr. 2019
 *      Author: mattias
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include "common.h"

using namespace std;

string trim(string str) {
	auto ret = str;
	for (int i = 0; i < str.size(); ++i) {
		if (!isspace(str[i])) {
			ret.erase(ret.begin(), ret.begin() + i);
			break;
		}
	}

	while (!ret.empty() && isspace(ret[ret.size() - 1])) {
		ret.pop_back();
	}
	ret.shrink_to_fit();
	return ret;
}

pair<string, string> split(string str, char delimiter) {
	pair<string, string> ret;

	auto f = str.find(delimiter);
	if (f != string::npos) {
		ret.first.assign(str.begin(), str.begin() + f);
		ret.second.assign(str.begin() + f + 1, str.end());
	}
	else {
		ret.first = str;
	}

	return ret;
}

int main(int argc, char **argv) {
	string filename = "";
	if (argc > 1) {
		filename = argv[1];
	}
	else {
		cout << "usage vbmake [file]" << endl;
		return 0;
	}

	fstream projectFile(filename);

	set<string> files;

	if (!projectFile.is_open()) {
		cerr << "could not open file " << filename << endl;
		return -1;
	}

	auto addFile = [&files] (string file) {
		if (file.empty()) {
			cout << "tried to add empty filename " << endl;
			return;
		}
		files.insert(file);
	};

	cout << "# Buildfile for Lasersköld vbconv for project file '" << filename << "'" << endl;

	for (string line; getline(projectFile, line); ) {
//		cout << "line" << line << endl;
		auto s = split(line, '=');
		s.first = trim(s.first);

//		cout << "assigning " << trim(s.second) << " to " << trim(s.first) << endl;

		auto t = split(s.second, ';');
		auto filename = trim(t.second);
		if (filename.empty()) {
			filename = trim(t.first);
		}
		if (s.first == "Module") {
//			cout << "Module with name " << filename << endl;
			addFile(filename);
		}
		else if (s.first == "Class") {
//			cout << "Class with name" << filename << endl;
			addFile(filename);
		}
		else if (s.first == "Form") {
//			cout << "Form with name " << filename << endl;
			addFile(filename);
		}
	}

	string filelist;
	for (auto file: files) {
		auto unitName = getUnitName(file);
		filelist += " " + toLower(unitName + ".o");
	}
	cout << "program: " << filelist << endl;
	cout << "\tg++ -o program " << filelist << endl;

	for (auto file: files) {
		string references;
		if (file.size() > 1) {
			for (auto f: files) {
				if (f != file) {
					references += " " + f;
				}
			}
		}

//		string unitName = getUnitName(file);
//		string lcase  = toLower(unitName);
//		string compiler = getDirectory(argv[0]) + "/" + "cppgen";
//		string headerCommand = compiler + " " + file + " -o " + lcase + ".h --header --references " + references;
//		string sourceCommand = compiler + " " + file + " -o " + lcase + ".cpp --source --references " + references;
//		string headerRule = lcase + ".h: " + file + references + "\n\t" + headerCommand;
//		string sourceRule = lcase + ".cpp: " + file + " " + lcase + ".h" + "\n\t" + sourceCommand;
//		string buildRule = lcase + ".o: " + lcase + ".cpp";
//		cout << headerRule << endl;
//		cout << sourceRule << endl;
//		cout << buildRule << endl << endl;
	}
}


