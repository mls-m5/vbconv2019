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
	string filename = "originals/SD/Ship.cls";
//	string filename = "originals/SD/MainMod.bas";
//	string filename = "originals/SD/MapMod.bas";
//	string filename = "originals/SD/MarketMod.bas";
//	string filename = "originals/SD/Shot.cls";
	loadTypeInformation(getDirectory(filename));
	File f(filename);


	auto output = generateCpp(f.tokens);

	auto ending = getEnding(filename);

	auto unitName = getFileName(filename);

	string headerString =  "// Generated with Lasersköld vb6conv cpp-generator\n\n";
	if (ending == "cls") {
		output.children.insert(output.begin(), Group({Token("class " + unitName + " {\npublic:\n", output.location())}));
		output.push_back(Token("};\n", output.location()));
	}
	else if (ending == "bas") {
		unitName = string(unitName.begin(), unitName.end() - 4);
		output.children.insert(output.begin(), Token("namespace " + unitName + " {\n", output.location()));
		output.push_back(Token("} // namespace \nusing namespace " + unitName + ";\n", output.location()));
	}
	output.children.insert(output.children.begin(), Token(headerString + "#include \"vbheader.h\"\n\n", output.location()));

	cout << output.spelling() << endl;
}
