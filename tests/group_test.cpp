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
	TestFile f(testCode1);

	f.tokens.printRecursive(cout, 0);
}


TEST_CASE("binary expression") {
	stringstream ss("test + x * y ^ 2 + 6");
	File f;
	f.load(ss, "test");

	cout << "result: " << endl;
	f.tokens.printRecursive(cout);
	ASSERT_EQ(f.tokens.front().type(), Token::AdditionOrSubtraction);
	ASSERT_EQ(f.tokens.size(), 1);
}


TEST_CASE("type characters") {
	TestFile f("x!");

	f.tokens.printRecursive(cout);
	ASSERT_EQ(f.tokens.front().type(), Token::TypeCharacterClause);
}


TEST_CASE("property accessor") {
	TestFile f("apa.bepa");

	f.tokens.printRecursive(cout);

	ASSERT_EQ(f.tokens.front().type(), Token::PropertyAccessor);
}


TEST_CASE("function call") {
	TestFile f("x.y( z )");
	f.tokens.printRecursive(cout, 0);
}

TEST_CASE("Coma list") {
	TestFile f("1, 2, 3");
	f.tokens.printRecursive(cout);
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("Public variable list") {
	TestFile f("Public XPos#, YPos#, Angle!");
	f.tokens.printRecursive(cout);
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("z - file load test") {
	try {
		File f("Ship.cls");
		f.tokens.printRecursive(cout, 0);
	}
	catch (VerificationError &e) {
		cout << e.what() << endl;
		ERROR("verification failed");
	}
}


TEST_SUIT_END
