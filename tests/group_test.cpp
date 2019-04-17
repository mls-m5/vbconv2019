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
		tokens.printRecursive(cout);
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
		elseif y > z then
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

//	f.tokens.printRecursive(cout, 0);
}


TEST_CASE("binary expression") {
	TestFile f("x = test + x * y ^ 2 + 6");

	ASSERT_EQ(f.tokens.front().type(), Token::Assignment);
	ASSERT_EQ(f.tokens.size(), 1);
}


TEST_CASE("type characters") {
	TestFile f("x!");

//	f.tokens.printRecursive(cout);
	ASSERT_EQ(f.tokens.front().type(), Token::TypeCharacterClause);
}


TEST_CASE("property accessor") {
	TestFile f("apa.bepa");

//	f.tokens.printRecursive(cout);

	ASSERT_EQ(f.tokens.front().type(), Token::FunctionCallOrPropertyAccessor);
}

TEST_CASE("empty Comma list") {
	TestFile f("x 10, , , , 4");
	ASSERT_EQ(f.tokens.size(), 1);
	ASSERT_EQ(f.tokens.front().type(), Token::MethodCall);
}

TEST_CASE("byref test") {
	TestFile f("Sub main (Byref y)");
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("mixed members and function") {
	TestFile f("x = Ammo / Weapons.Weapons(WeaponSelected).Ammo");
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("optional byval") {
	TestFile f("optional byval x as string");
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("for loop") {
	{
		TestFile f("for i as single = 0 to 10");
		ASSERT_EQ(f.tokens.size(), 1);
		ASSERT_EQ(f.tokens.front().type(), Token::ForLoop);
	}
	{
		TestFile f("for i as single = 0 to 10 step 2");
		ASSERT_EQ(f.tokens.size(), 1);
		ASSERT_EQ(f.tokens.front().type(), Token::ForLoop);
	}
}

TEST_CASE("function call") {
	TestFile f("x.y( z )");
//	f.tokens.printRecursive(cout, 0);
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("set operation") {
	TestFile  f("set x = y");
	ASSERT_EQ(f.tokens.size(), 1);
	ASSERT_EQ(f.tokens.front().type(), Token::SetStatement);
}

TEST_CASE("default as clause") {
	TestFile f("x as long = 1");
	ASSERT_EQ(f.tokens.size(), 1);
	ASSERT_EQ(f.tokens.front().type(), Token::DefaultAsClause);
}

TEST_CASE("negation") {
	{
		TestFile f("x = -1");
		ASSERT_EQ(f.tokens.size(), 1);
		ASSERT_EQ(f.tokens.front().type(), Token::Assignment);
		ASSERT_EQ(f.tokens.front().back().type(), Token::UnaryIdentityOrNegation);
	}

	{
		TestFile f("x = 1 + - 1");
		ASSERT_EQ(f.tokens.size(), 1);
		ASSERT_EQ(f.tokens.front().type(), Token::Assignment);
		ASSERT_EQ(f.tokens.front().back().type(), Token::AdditionOrSubtraction);
	}

	{
		TestFile f("x = 2 - 1 / 2");
		ASSERT_EQ(f.tokens.size(), 1);
		ASSERT_EQ(f.tokens.front().type(), Token::Assignment);
		ASSERT_EQ(f.tokens.front().back().type(), Token::AdditionOrSubtraction);
	}

	{
		TestFile f("PrintInt - 1");
		ASSERT_EQ(f.tokens.size(), 1);
		ASSERT_EQ(f.tokens.front().type(), Token::MethodCall);
		ASSERT_EQ(f.tokens.front()[1].type(), Token::UnaryIdentityOrNegation);
	}
}

TEST_CASE("logical operators") {
	TestFile f("x = y and z or w xor b");
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("comment after declaration") {
	TestFile f("Dim x! 'This is a comment");

	ASSERT_EQ(f.tokens.size(), 1);
	ASSERT_EQ(f.tokens.front().type(), Token::DimStatement);
}

TEST_CASE("Comma list") {
	TestFile f("1, 2, 3");
	ASSERT_EQ(f.tokens.size(), 1);
}

TEST_CASE("Public variable list") {
	TestFile f("Public XPos#, YPos#, Angle!");
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
