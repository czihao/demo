#pragma once
#include "FullCalendarBuilder.h"
#include "DisplayableComponentDecorator.h"
#include<string>

class fullCalendarBuilder;

class ExternalEventDecorator : public DisplayableComponentDecorator {
	friend FullCalendarBuilder;
public:
	ExternalEventDecorator(std::shared_ptr<DisplayableComponent>, std::string, std::tm, std::string);
	void display(int depth);
public:
	std::string calendar_name;
	std::tm when;
	std::string event_name;
};