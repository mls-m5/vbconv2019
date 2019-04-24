/*
 * main.cpp
 *
 *  Created on: 19 apr. 2019
 *      Author: mattias
 */

#include "gencpp.h"
#include "typelibrary.h"
#include "file.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace vbconv;
using namespace std;

int main(int argc, char **argv) {
	string filename;
	string outputFile = "out.cpp";
	bool outputHeader = false;
	bool outputSource = false;
//	string filename = "originals/SD/Ship.cls";
//	string filename = "originals/SD/MainMod.bas";
//	string filename = "originals/SD/MapMod.bas";
//	string filename = "originals/SD/MarketMod.bas";
//	string filename = "originals/SD/Shot.cls";

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
			else if (i + 1 < argc) {
				if (arg == "-o") {
					outputFile = argv[++i];
				}
			}
		}
		if (!outputHeader && !outputSource) {
			outputHeader = true;
		}
	}
	else {
		cout << "usage\n vbgen [vbfile] [options]" << endl;
		cout << "example options" << endl;
		cout << "--header      output c++ header (.h)" << endl;
		cout << "--source      output c++ source (.cpp)" << endl;
		cout << "--all         output .h and .cpp files" << endl;
		cout << "--o           set location of output file" << endl;
		cout << "--o -         output to std out" << endl;
		return 0;
	}

	if (outputHeader) {
		auto output = generateCpp(filename, true);
		if (output == "-" || (outputHeader && outputSource)) {
			cout << output.spelling() << endl;
		}
		else {
			ofstream file(outputFile);
			file << output.spelling() << endl;
		}
	}

	if (outputSource) {
		auto output = generateCpp(filename, false);
		if (output == "-") {
			cout << output.spelling() << endl;
		}
		else {
			ofstream file(outputFile);
			file << output.spelling() << endl;
		}
	}
}
