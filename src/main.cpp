
#include "file.h"
#include <iostream>

using namespace std;
using namespace vbconv;


int main(int argc, char **argv) {
	cout << "Hello" << endl;
	
	File file("originals/SD/Ship.cls");

	for (Tokens &token: file.tokens) {
		cout << token.token << endl;
	}

	auto stripR = [] (string &s) {
		if (s.empty()) {
			return;
		}
		for (int i = 0; i < s.size(); ) {
			if (s[i] == '\r') {
				s.erase(i, 1);
			}
			else {
				++i;
			}
		}
	};

	for (Tokens &token: file.tokens) {
		auto s =token.token.spelling();
		stripR(s);
		cout << s;
	}

	cout << endl;

	return 0;
}


