#pragma once
#include<memory>
#include<vector>
#include "DisplayableComponent.h"

// forward declaration, needed for friend declaration
class FullCalendarBuilder;
class CalendarInterface;
class CalendarNameDecorator;

class DisplayableComponentDecorator : public DisplayableComponent {
	friend FullCalendarBuilder;
	friend CalendarInterface;
	friend CalendarNameDecorator;
public:
	std::shared_ptr<DisplayableComponent> _component;
	DisplayableComponentDecorator(std::shared_ptr<DisplayableComponent> _component);
	void display(int depth);
protected:
	std::string calendar_name;
};
