/*
 * form.cpp
 *
 *  Created on: 28 apr. 2019
 *      Author: Mattias Larsson Sköld
 */

#include <matgui/window.h>
#include <matgui/paint.h>
#include "../../include/Form.h"
#include "iostream"
#include "vbheader.h"

using namespace MatGui;
using namespace std;

struct _FormImpl: public Window {
	Form *form;
	_FormImpl(Form *form): Window("Form", 600, 400, true), form(form) {
		linePaint.line.color(1,1,1);
		closeSignal.connect([this]() {
			short ret = 0;
			this->form->Form_Unload(ret);
			return ret;
		});
	}

	Paint linePaint;
	bool isLoaded = false;

	struct Line {
		Line() = default;
		Line(const Line&) = default;
		Line(Line &&) = default;
		Line(double x1, double y1, double x2, double y2): x1(x1), y1(y1), x2(x2), y2(y2) {}

		double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	};

	std::vector<Line> lines;

	void draw() override {
		Window::draw();
		for (auto &l: lines) {
			linePaint.drawLine(l.x1, l.y1, l.x2, l.y2);
		}
	}

	void cls() {
		lines.clear();
	}
};

Form::Form(): impl(new _FormImpl(this)) {
	impl->keyDown.connect([this](View::KeyArgument arg) {
		this->Form_KeyDown(arg.symbol, arg.modifier);
	}, this);

	impl->pointerMoved.connect([this](View::PointerArgument arg) {
		this->Form_MouseMove(arg.state, 0, arg.x, arg.y);
	});

	impl->pointerDown.connect([this](View::PointerArgument arg) {
		this->Form_MouseDown(arg.state, 0, arg.x, arg.y);
	});

	impl->pointerUp.connect([this](View::PointerArgument arg) {
		this->Form_MouseUp(arg.state, 0, arg.x, arg.y);
	});
}

void Form::Show() {
	if (!impl->isLoaded) {
		impl->isLoaded = true;
		Form_Load();
	}
	impl->show();
}


void Form::Line(double x1, double y1, double x2, double y2) {
//	impl->linePaint.drawLine(x1, y1, x2, y2);
	impl->lines.emplace_back(x1, y1, x2, y2);
}

void Form::Cls() {
	impl->cls();
}
void Form::Print(VB::VBString text) {
	// Todo: Implement this
	cout << text << endl;
}

Form::~Form() {
}




