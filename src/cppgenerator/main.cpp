/*
 * main.cpp
 *
 *  Created on: 19 apr. 2019
 *      Author: mattias
 */

#include "common.h"
#include "gencpp.h"
#include "typelibrary.h"
#include "file.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace vbconv;
using namespace std;

string createDependencyString(string base, vector<string> dependencies) {
	string ret;

	if (dependencies.empty()) {
		return {};
	}

	ret = base + ":";

	for (auto &dependency: dependencies) {
		ret += " " + dependency;
	}

	ret += "\n";

	return ret;
}


int main(int argc, char **argv) {
	string filename;
	string outputFile = "out.cpp";
	bool outputHeader = false;
	bool outputSource = false;
	bool isReferencesSpecified = false;
	bool byvalRefType = true;
//	bool byvalRefType = false;

	if (argc > 1) {
		filename = argv[1];
		auto ending = getEnding(filename);
		if (!ending.empty()) {
			outputFile = string(filename.begin(), filename.end() - ending.size() - 1);
		}

		for (int i = 2; i < argc; ++i) {
			string arg = argv[i];
			if (arg == "--header") {
				outputHeader = true;
			}
			else if (arg == "--source") {
				outputSource = true;
			}
			else if (arg == "--all") {
				outputHeader = outputSource = true;
			}
			else if (arg == "-v") {
				setVerboseOutput(true);
			}
			else if (arg == "-d") {
				setVerboseOutput(true);
				setDebugOutput(true);
			}
			else if (arg == "--byval") {
				byvalRefType = true;
			}
			else if (arg == "--byref") {
				byvalRefType = false;
			}
			else if (arg == "-l") {
				setInsertLineNumberReference(true);
			}
			else if (i + 1 < argc) {
				if (arg == "-o") {
					outputFile = argv[++i];
				}
				if (arg == "--references") {
					for (++i; i < argc; ++i) {
						isReferencesSpecified = true;
						loadTypeInformation(argv[i]);
					}
				}
			}
		}
		if (!outputHeader && !outputSource) {
			outputHeader = true;
			outputSource = true;
		}
	}
	else {
		cout << "usage\n vbgen [vbfile] [options]" << endl;
		cout << "example options" << endl;
		cout << "--header            output c++ header (.h)" << endl;
		cout << "--source            output c++ source (.cpp)" << endl;
		cout << "--all               output .h and .cpp files" << endl;
		cout << "-o                  set location of output file" << endl;
		cout << "-o ./               set location of output file" << endl;
		cout << "-o -                output to std out" << endl;
		cout << "-v                  show extra text output" << endl;
		cout << "-d                  show information interesting for developers" << endl;
		cout << "--references [...]  specify which objects to link together (standard is all files in folder)" << endl;
		cout << "--byval             switch to byval as standard instead of byref" << endl;
		cout << "--byref             switch to byval as standard" << endl;
		cout << "-l                  insert g++ line hints" << endl;
		return 0;
	}

	vout << "Creating cpp file for " << filename << endl;

	if (!isReferencesSpecified) {
		loadTypeInformation(getDirectory(filename));
	}
	else {
		loadTypeInformation(filename);
	}

	vector <Group *> referencedFunctions;

	auto outputSourceFunction = [&] (bool headerMode) {
			auto output = generateCpp(filename, headerMode, byvalRefType);

			auto f1 = output.getByType(Token::CVoidFunction);
			referencedFunctions.insert(referencedFunctions.end(), f1);
			auto f2 = output.getByType(Token::CFunctionWithType);
			referencedFunctions.insert(referencedFunctions.end(), f2);

			if (outputFile == "-") {
				cout << output.spelling() << endl;
			}
			else {
				string outputFileEnding = headerMode? ".h": ".cpp";
				ofstream file(stripEnding(outputFile) + outputFileEnding);
				if (!file.is_open()) {
					cerr << "could not open file " << outputFile + outputFileEnding << endl;
					throw "error";
				}
				file << output.spelling() << endl;
			}
	};

	if (outputHeader) {
		outputSourceFunction(true);
	}



	vector<Group> &extractedSymbols = getExtractedSymbols();

	ofstream dependencyFile;

	if (outputFile != "-") {
		dependencyFile.open(outputFile + ".d");
	}


	auto printSymbols = [&](Group &g, ostream &stream, string name) {
		// Creates a header for a single type or a enum
		stream << "#pragma once" << endl;
		stream << "#include \"vbheader.h\"" << endl;
		stream << endl;
		vector<string> dependencies;
		for (auto r: g.getAllByType(Token::CReference)) {
			auto typeDeclaration = findTypeDeclaration(r->token);
			if (typeDeclaration) {
				auto unitName = typeDeclaration->sourceUnit;
				vout << r->spelling() << " in " << unitName << endl;
//				auto dependencyName = toLower(unitName + "-" + r->spelling()) + ".h";
				auto dependencyName = unitName + "-" + r->spelling() + ".h";
				if (typeDeclaration->type == ScopeType::Class) {
//					dependencyName = toLower(unitName) + ".h";
					dependencyName = unitName + ".h";
				}
				stream << "#include \"" + dependencyName + "\"\n" << endl;
				dependencies.push_back(dependencyName);
			}
		}
		stream << g.spelling() << endl;
		if (dependencyFile.is_open() && !name.empty()) {
			dependencyFile << createDependencyString(name, dependencies);
		}
	};

	if (outputFile == "-") {
		vout << "headerfiles:" << endl;
		for (auto &symbol: extractedSymbols) {
			printSymbols(symbol, cout, "");
		}
	}
	else {
		auto unitName = getUnitName(outputFile);
		std::vector<string> symbolStrings = getUnitReferences();
		symbolStrings.reserve(symbolStrings.size() + extractedSymbols.size());
		for (auto &symbol: extractedSymbols) {
			auto symbolNameGroup = symbol.getByType(Token::CSymbolName);
//			auto symbolOutputFile = getDirectory(outputFile) + unitName + "-" +  toLower(symbolNameGroup->spelling()) + ".h";
			auto symbolOutputFile = getDirectory(outputFile) + unitName + "-" +  symbolNameGroup->spelling() + ".h";
			vout << "-- outputting to file --> " << symbolOutputFile << endl;
			ofstream file(symbolOutputFile);
			if (!file.is_open()) {
				cerr << "could not open file " << symbolOutputFile << endl;
			}
			printSymbols(symbol, file, symbolOutputFile);

			symbolStrings.push_back(symbolOutputFile);
			symbolStrings.push_back(filename);
		}

		dependencyFile << createDependencyString(outputFile, symbolStrings) << endl;
	}

	if (outputSource) {
		outputSourceFunction(false);
//		auto output = generateCpp(filename, false, byvalRefType);
//
//		auto f1 = output.getByType(Token::CVoidFunction);
//		referencedFunctions.insert(referencedFunctions.end(), f1);
//		auto f2 = output.getByType(Token::CFunctionWithType);
//		referencedFunctions.insert(referencedFunctions.end(), f2);
//
//		if (outputFile == "-") {
//			cout << output.spelling() << endl;
//		}
//		else {
//			ofstream file(stripEnding(outputFile) + ".cpp");
//			file << output.spelling() << endl;
//		}
	}
}
