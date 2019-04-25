/*
 * gencpp.h
 *
 *  Created on: 23 apr. 2019
 *      Author: mattias
 */


#include "token.h"
#pragma once

namespace vbconv {

// Generate cpp code (as a pseudo ast-tree) from a vb ast
Group generateCpp(const Group &g);
Group generateCpp(std::string filename, bool header);

// Specify if the output file is a header
// Default is true
void setHeaderMode(bool mode);

// Set the file name of the current code unit
void setFilename(std::string filename);

std::string getCurrentUnitName();

void addReferenceToClass(std::string className);
void addReferenceToType(std::string className);

void setVerboseOutput(bool state);
extern bool verboseOutput;

// Get types and enums that is supposed to be extracted to
// separate files
std::vector<Group> &getExtractedSymbols();
std::vector<std::string> &getUnitReferences();

}


#define vout if (vbconv::verboseOutput) cout
