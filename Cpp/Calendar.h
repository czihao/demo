#pragma once

#include "CalendarComponent.h"
// forward declarations needed to avoid circular includes, used to declare friends only
class FullCalendarBuilder;
class CalendarInterface;

class Calendar : public CalendarComponent {
	// friends
	friend FullCalendarBuilder;
	friend CalendarInterface;
protected:
	std::string name;
	size_t yearsToHold;
	std::tm currentDate; // current date and time
						 // dateInfo is the start date and time

						 // Calendar contains years, so override accordingly
	virtual std::shared_ptr<DisplayableComponent> addComponent(std::shared_ptr<DisplayableComponent>) override;
public:
	// arguments: name of the calendar, length of the calendar in years
	Calendar(std::string n, size_t y);
	// inherited methods
	virtual void display(int depth) override;

};