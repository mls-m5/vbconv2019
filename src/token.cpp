/*
 * token.cpp
 *
 *  Created on: 14 apr. 2019
 *      Author: Mattias Lasersköld
 */


#include "token.h"
#include "file.h"
#include <iostream>

namespace vbconv {


std::ostream& operator <<(std::ostream& stream, Token& t) {
	auto location = t.extent.begin;
	stream << location.file->filename << ":" << location.line << ":"
			<< location.col << ":" << t.c_str();
	return stream;
}

void Tokens::group() {


}




}  // namespace name
