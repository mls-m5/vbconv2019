/*
 * gencpp.h
 *
 *  Created on: 23 apr. 2019
 *      Author: mattias
 */


#include "token.h"
#pragma once

namespace vbconv {

Group generateCpp(const Group &g);

extern bool verboseOutput;

}


#define vout if (vbconv::verboseOutput) cout
