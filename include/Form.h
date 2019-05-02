/*
 * form.h
 *
 *  Created on: 28 apr. 2019
 *      Author: Mattias Larsson Sköld
 */

#pragma once

#include <memory>
#include "vbheader.h"

class Form {
public:
	Form();
	virtual ~Form();
	virtual void Form_KeyDown(short KeyCode, short Shift) {}
	virtual void Form_Load() {}
	virtual void Form_MouseDown(short Button, short Shift, float X, float Y) {}
	virtual void Form_MouseMove(short Button, short Shift, float X, float Y) {}
	virtual void Form_MouseUp(short Button, short Shift, float X, float Y) {}
	virtual void Form_Unload(short &Cancel) {}

	// Shows the form and calls Form_Load if the form has not been loaded before
	void Show();
	void Line(double x1, double y1, double x2, double y2);
	void Cls();
	void Print(VB::VBString text);

	std::unique_ptr<struct _FormImpl> impl;
};
