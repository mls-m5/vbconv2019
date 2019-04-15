/*
 * group_test.cpp
 *
 *  Created on: 14 apr. 2019
 *      Author: Mattias Lasersköld
 */


#define DO_NOT_CATCH_ERRORS
#include "file.h"
#include "mls-unit-test/unittest.h"
#include <sstream>

using namespace std;
using namespace vbconv;

File createAsFile(string str) {
	File f;

	stringstream ss(str);

	f.load(ss, "stdin");

	return move(f);
}

const char *testCode1 = R"_(
class test

	sub init ()
		print "hej"

	end sub

	sub move (x as single, y as single)
		print "hej"
	end sub	

end class


)_";


TEST_SUIT_BEGIN


TEST_CASE("test") {
//	auto f = createAsFile(testCode1);
	stringstream ss(testCode1);
//	File f(ss, string("stdin"));
	File f;
	f.load(ss, "test");

	f.tokens.printRecursive(cout, 0);

	cout << "=== starting test output" << endl;

	cout << f.tokens.children.front().spelling() << endl;

	for (int i = 0; i < f.tokens.children.size(); ++i) {
		cout << f.tokens.children[i].spelling() << endl;
	}

	ASSERT_EQ(f.tokens.children.size(), 8);
}



TEST_SUIT_END
