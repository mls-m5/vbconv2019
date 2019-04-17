/*
 * basic_element_test.cpp
 *
 *  Created on: 17 apr. 2019
 *      Author: Mattias Larsson Sköld
 */


#include "mls-unit-test/unittest.h"
#include "pattern.h"

using namespace std;
using namespace vbconv;

TEST_SUIT_BEGIN


TEST_CASE("token exclude rule") {
	TokenPattern pattern({Token::Any}, {Token::Then});

	ASSERT(!(pattern == Token::Then), "Rule does not work properly");
}

TEST_SUIT_END

