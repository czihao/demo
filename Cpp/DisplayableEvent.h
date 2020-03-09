#pragma once
#include "CalendarComponent.h"
#include "DisplayableDay.h"
// forward declarations needed to avoid circular includes, used to declare friends only
class FullCalendarBuilder;
class CalendarInterface;
class DisplayableMonth;
class DisplayableDay;

class DisplayableEvent : public CalendarComponent {
	friend class DisplayableMonth;
	friend class DisplayableDay;
	friend FullCalendarBuilder;
	friend CalendarInterface;
public:
	// 1st argument = start date/timeof the day, 2nd argument = its parent
	DisplayableEvent(std::tm, std::shared_ptr<DisplayableComponent>, std::string, std::string, int, int);
	virtual void display(int depth) override;
protected:
	std::string event_name;
	std::string calendar_name;
	int recurEvery;
	int recurFor;
};

