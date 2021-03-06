
#include "file.h"
#include <iostream>

using namespace std;
using namespace vbconv;


int main(int argc, char **argv) {
	cout << "Hello" << endl;
	
	string filename = "";
	if (argc > 1) {
		filename = argv[1];
	}
	else {
		cout << "VB ast explorer" << endl;
		cout << "usage: ast [filename]" << endl;
		return 0;
	}

	File file(filename);

	auto &groups = file.tokens;
//	for (int i = 0; i < groups.size(); ++i) {
//		cout << groups[i].token << endl;
//	}
//
//	cout << "compact version" << endl;
//	for (int i = 0; i < groups.size(); ++i) {
//		cout << groups[i].concatSmall() << endl;
//	}

	cout << "recursive print" << endl;
	groups.printRecursive(cout, 0);

	cout << "raw:" <<  endl;

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

	for (Group &group: file.tokens) {
		auto s =group.spelling();
		stripR(s);
		cout << s;
	}

	cout << endl;

	return 0;
}


