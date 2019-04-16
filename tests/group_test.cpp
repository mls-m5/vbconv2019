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

	sub move (x as single, y as single, z as long)
		if x > y then
			print "hej"
		elif y > z then
			print "da"
		else
			print "re"
		end if
	end sub	

end class


)_";


TEST_SUIT_BEGIN


TEST_CASE("test") {
	stringstream ss(testCode1);
	File f;
	f.load(ss, "test");

	f.tokens.printRecursive(cout, 0);

//	cout << "=== starting test output" << endl;
//
//	cout << f.tokens.children.front().spelling() << endl;
//
//	for (int i = 0; i < f.tokens.children.size(); ++i) {
//		cout << f.tokens.children[i].spelling() << endl;
//	}
//
//	ASSERT_EQ(f.tokens.children.size(), 8);
}

TEST_CASE("binary expression") {
	stringstream ss("test + x * y ^ 2");
	File f;

	f.load(ss, "test");
	cout << "result: " << endl;
	f.tokens.printRecursive(cout, 0);
}


TEST_SUIT_END
