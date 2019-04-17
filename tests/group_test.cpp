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

class TestFile: public File {
public:
	TestFile(string code) {
		stringstream ss(code);
		load(ss, "test");
	}
};

const char *testCode1 = R"_(
class test

	Private Sub init ()
		print "hej"

	End Sub

	Public sub move (x as single, y as single, z as long)
		if x > y then
			print "Hej"
		elif y > z then
			print "da"
		else
			print "re"
		end if

	end sub	

	Private Sub Do2 ()
	End Sub

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
	ASSERT_EQ(f.tokens.front().type(), Token::AdditionOrSubtraction);
}

TEST_CASE("property accessor") {
	TestFile f("apa.bepa");

	f.tokens.printRecursive(cout, 0);

	ASSERT_EQ(f.tokens.front().type(), Token::PropertyAccessor);
}

TEST_CASE("function call") {
	TestFile f("x.y( z )");
	f.tokens.printRecursive(cout, 0);
}

TEST_CASE("file load test") {
	File f("Ship.cls");
	f.tokens.printRecursive(cout, 0);
}


TEST_SUIT_END
