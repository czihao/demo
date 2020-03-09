#pragma once
/*
Author: Jon Shidal
Purpose: declare the DisplayableMonth class. This class may be a component of a calendar. It inherits from CalendarComponent and defines pure virtual functions
declared in DisplayableComponent
*/

#include "CalendarComponent.h"
// forward declarations needed to avoid circular includes, used to declare friends only
class FullCalendarBuilder;
class CalendarInterface;
class DisplayableYear;

class DisplayableMonth : public CalendarComponent {
	friend class DisplayableYear;
	friend class DisplayableEvent;
	friend FullCalendarBuilder;
	friend CalendarInterface;
public:
	// arguments = date/time info, its parent, name of the month, days in the month
	DisplayableMonth(std::tm, std::shared_ptr<DisplayableComponent>, std::string monthname, unsigned int numdays);
	virtual void display(int depth) override;
protected:
	std::string name;
	unsigned int numberOfDays;
	// Month contains days, so it is a composite object. override addComponent accordingly
	virtual std::shared_ptr<DisplayableComponent> addComponent(std::shared_ptr<DisplayableComponent>) override;
};