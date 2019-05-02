/*
 * Main.cpp
 *
 *  Created on: 28 apr. 2019
 *      Author: mattias
 */



// namespace MainMod { void Main(); } using namespace MainMod;

#include <matgui/application.h>
#include <matgui/matsig.h>
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include "vbheader.h"

using namespace std;
using namespace MatGui;

static Application *application;

int main(int argc, char **argv) {
	Application app(argc, argv);
	application = &app;
	app.ContinuousUpdates(false);

#ifdef VB_STARTUP_FUNCTION
	cout << "Running main from module" << endl;
	VB_STARTUP_FUNCTION;
#else

#ifdef VB_STARTUP_FORM
//	cout << "starting form " << #VB_STARTUP_FORM# << endl;
	auto form = make_shared<VB_STARTUP_FORM>();
	form->Show();
	application->mainLoop();
#else
	#error"no startup defined";
#endif
#endif
}

namespace VB {

void DoEvents() {
	application->handleEvents();
	MatSig::flushSignals();
	SDL_Delay(10);
}

bool FileExists(VBString filename) {
	struct stat file_stat;
//	int err = stat(filename.c_str(), &file_stat);
//    if (err != 0) {
//    	return false;
//    }
//    return true;
	return (stat(filename.c_str(), &file_stat) == 0);
}

void MessageBox(VBString text, VBString title) {
	cout << text << endl;
}

VBString InputBox(VBString text, VBString title, VBString defaultValue) {
	if (!title.empty()) {
		cout << title << endl;
		cout << string(title.length(), '=') << endl;
	}
	cout << text << endl;
	VBString response;

	cin >> response;
	return response;
}

}  // namespace VB
