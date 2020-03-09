#include "stdafx.h"
#include "ExternalEventDecorator.h"
#include "DisplayableComponentDecorator.h"
#include <iostream>

using namespace std;

ExternalEventDecorator::ExternalEventDecorator(std::shared_ptr<DisplayableComponent> _component, string cal_name, tm d, string event) : DisplayableComponentDecorator(_component), calendar_name(cal_name), when(d), event_name(event){}

void ExternalEventDecorator::display(int depth) {
	if (depth == month) {

	}
	else if(depth == year || depth == day) {
		cout << calendar_name << "::";
	}
	else if (depth == events) {
		cout << "\t";
		cout << calendar_name << "::";
	}
	
	DisplayableComponentDecorator::display(depth);
}
