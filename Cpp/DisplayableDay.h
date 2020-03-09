#pragma once
/*
Author: Jon Shidal
Purpose: declare the DisplayableDay class. This class may be a component of a calendar. It inherits from CalendarComponent and defines pure virtual functions
declared in DisplayableComponent
*/

#include "CalendarComponent.h"
#include "DisplayableEvent.h"
// forward declarations needed to avoid circular includes, used to declare friends only
class FullCalendarBuilder;
class CalendarInterface;
class DisplayableMonth;

class DisplayableDay : public CalendarComponent {
	friend class DisplayableMonth;
	friend class DisplayableEvent;
	friend FullCalendarBuilder;
	friend CalendarInterface;
public:
	// 1st argument = start date/timeof the day, 2nd argument = its parent
	int events_in_day;
	DisplayableDay(std::tm, std::shared_ptr<DisplayableComponent>);
	void Display_event(int depth); //Helps reduce code
	virtual void display(int depth) override;
	virtual std::shared_ptr<DisplayableComponent> addComponent(std::shared_ptr<DisplayableComponent>) override;
};