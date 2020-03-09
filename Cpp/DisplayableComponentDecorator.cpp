#include "stdafx.h"
#include "DisplayableComponentDecorator.h"
#include "DisplayableEvent.h"
#include <iostream>

using namespace std;


DisplayableComponentDecorator::DisplayableComponentDecorator(std::shared_ptr<DisplayableComponent> _component) : _component(_component), DisplayableComponent(_component) {}

void DisplayableComponentDecorator::display(int depth) {
	if (dynamic_cast<DisplayableEvent*>(_component.get())) {
		_component->display(depth);
	}

}