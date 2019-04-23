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

using namespace vbconv;
using namespace std;

int main(int argc, char **argv) {
//	string filename = "originals/SD/Ship.cls";
	string filename = "originals/SD/MainMd.bas";
//	string filename = "originals/SD/MapMod.bas";
	loadTypeInformation(getDirectory(filename));
	File f(filename);

	cout << "// Lasersköld vb6conv cpp-generator" << endl;

	auto output = generateCpp(f.tokens);

	auto ending = getEnding(filename);

	auto unitName = getFileName(filename);

	output.children.insert(output.children.begin(), Token("#include \"vbheader.h\"", output.location()));
	if (ending == "cls") {
		output.children.insert(output.begin(), Group({Token("class " + unitName + " {\n", output.location())}));
		output.push_back(Token("};", output.location()));
	}

	cout << output.spelling() << endl;
}
