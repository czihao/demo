#pragma once
#include "CalendarComponent.h"

class FullCalendarBuilder;
class CalendarInterface;
class DisplayableYear;

class DisplayableTask : public CalendarComponent {
	friend FullCalendarBuilder;
	friend CalendarInterface;
public:
	DisplayableTask(std::tm, std::shared_ptr<DisplayableComponent>, std::string, bool complete);
	virtual void display(int depth) override;
	void complete_task();
protected:
	std::string task_name;
	bool complete;
};