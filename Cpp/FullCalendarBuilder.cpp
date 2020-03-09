/*
Author: Jon Shidal
Purpose: Define full calendar builder class, responsible for building and managing the structure of the calendar
*/
#include "stdafx.h"
#include "FullCalendarBuilder.h"
#include "CalendarComponent.h"
#include "DisplayableEvent.h"
#include "DisplayableDay.h"
#include "DisplayableMonth.h"
#include "DisplayableYear.h"
#include "DisplayableTODOlist.h"
#include "DisplayableTask.h"
#include "ExternalEventDecorator.h"
#include <iostream>
using namespace std;

shared_ptr<Calendar> FullCalendarBuilder::buildCalendar(string name, size_t years) {
	currentCalendar = make_shared<Calendar>(name, years);
	// construct each year in a recursive way, add each year as a child of the calendar
	for (unsigned int i = 0; i < years; ++i) {
		tm y = currentCalendar->dateInfo;
		y.tm_year += i;
		y.tm_wday = (y.tm_wday + CalendarComponent::DAYSINAYEAR * i) % CalendarComponent::DAYSINAWEEK; // calculate day of the week for first day of the year
		currentCalendar->addComponent(buildYear(y, currentCalendar));
	}
	return currentCalendar;
}

shared_ptr<DisplayableComponent> FullCalendarBuilder::buildTODOlist() {
	shared_ptr<DisplayableComponent> todo_p = shared_ptr<DisplayableComponent>(DisplayableTODOlist::todolist(tm(), nullptr));
	return todo_p;
}

shared_ptr<DisplayableComponent> FullCalendarBuilder::buildTask(tm d, shared_ptr<DisplayableComponent> p, string task_name, bool complete) {
	shared_ptr<DisplayableComponent> task_p = make_shared<DisplayableTask>(d, p, task_name, complete);
	return task_p;
}

// Adding an event.
shared_ptr<DisplayableComponent> FullCalendarBuilder::buildEvent(shared_ptr<Calendar> cal, string name, tm when, int recurrEvery, int recurrFor, string calendar) {
	int depth = DisplayableComponent::depth;
	int days_in_mon;
	DisplayableMonth *mon_p;
	DisplayableEvent *event_p;
	shared_ptr<DisplayableComponent> evnt;
	for (int i = 0; i < recurrFor; ++i) {
		mon_p = dynamic_cast<DisplayableMonth *>(getComponentByDate(cal, when, "month").get());
		if (mon_p == nullptr)
		{
			printf("month index invalid\n");
			return nullptr;
		}
		days_in_mon = mon_p->numberOfDays;
		if (when.tm_mday > days_in_mon)
		{
			when.tm_mday %= days_in_mon;
			when.tm_mon += 1;
			if ((unsigned)when.tm_mon >= CalendarComponent::MONTHS)
			{
				when.tm_mon %= CalendarComponent::MONTHS;
				when.tm_year += 1;
			}
		}

		shared_ptr<DisplayableComponent> tmp_day = getComponentByDate(cal, when, "day");
		evnt = make_shared<DisplayableEvent>(when, tmp_day, name, calendar, recurrEvery, recurrFor);
		if (tmp_day == nullptr)
		{
			printf("day index invalid\n");
			return nullptr;
		}
		event_p = dynamic_cast<DisplayableEvent *>(evnt.get());
		tmp_day->addComponent(evnt);
		event_mm.insert(make_pair(event_p->event_name, evnt));
		when.tm_mday += recurrEvery;
	}
	DisplayableComponent::depth = depth;
	return evnt;
}

//Overloaded buildEvent method for when merging an external file
shared_ptr<DisplayableComponent> FullCalendarBuilder::buildEventMerge(string calendar_name, shared_ptr<Calendar> cal, string name, tm when, int recurrEvery, int recurrFor) {
	int depth = DisplayableComponent::depth;
	int days_in_mon;
	DisplayableMonth *mon_p;
	DisplayableEvent *event_p;
	shared_ptr<DisplayableComponent> evnt;
	for (int i = 0; i < recurrFor; ++i) {
		mon_p = dynamic_cast<DisplayableMonth *>(getComponentByDate(cal, when, "month").get());
		if (mon_p == nullptr)
		{
			printf("month index invalid\n");
			return nullptr;
		}
		days_in_mon = mon_p->numberOfDays;
		if (when.tm_mday > days_in_mon)
		{
			when.tm_mday %= days_in_mon;
			when.tm_mon += 1;
			if ((unsigned)when.tm_mon >= CalendarComponent::MONTHS)
			{
				when.tm_mon %= CalendarComponent::MONTHS;
				when.tm_year += 1;
			}
		}
		shared_ptr<DisplayableComponent> tmp_day = getComponentByDate(cal, when, "day");
		if (tmp_day == nullptr)
		{
			printf("day index invalid\n");
			return nullptr;
		}
		evnt = make_shared<DisplayableEvent>(when, cal, name, calendar_name, recurrEvery, recurrFor);
		event_p = dynamic_cast<DisplayableEvent *>(evnt.get());
		evnt = make_shared<ExternalEventDecorator>(evnt, calendar_name, when, name);
		event_mm.insert(make_pair(event_p->event_name, evnt));
		tmp_day->addComponent(evnt);
		when.tm_mday += recurrEvery;
		DisplayableComponent::depth = depth;
	}
	return evnt;
}

// Get the component of a calender object based on granularity
shared_ptr<DisplayableComponent> FullCalendarBuilder::getComponentByDate(shared_ptr<DisplayableComponent> cal, tm d, string granularity) {
	Calendar * cal_p = dynamic_cast<Calendar *>(cal.get());
	DisplayableYear *year_p = nullptr;
	DisplayableMonth *mon_p = nullptr;
	DisplayableDay *day_p = nullptr;
	int year_index = d.tm_year - cal_p->dateInfo.tm_year;
	int mon_index = d.tm_mon;
	int day_index = d.tm_mday - 1;
	//cout << year_index << endl << mon_index << endl << day_index << endl;
	shared_ptr<DisplayableComponent> cmpo_p;
	cmpo_p = cal_p->getChild(year_index);
	year_p = dynamic_cast<DisplayableYear *>(cmpo_p.get());
	if (year_p == nullptr)
	{
		cout << "error getting year from calender" << endl;
		return nullptr;
	}
	if (granularity == "year")
	{
		DisplayableComponent::depth = year;
		return cmpo_p;
	}

	cmpo_p = year_p->getChild(mon_index);
	mon_p = dynamic_cast<DisplayableMonth *>(cmpo_p.get());
	if (mon_p == nullptr)
	{
		cout << "error getting month from year" << endl;
		return nullptr;
	}
	if (granularity == "month")
	{
		DisplayableComponent::depth = month;
		return cmpo_p;
	}

	cmpo_p = mon_p->getChild(day_index);
	day_p = dynamic_cast<DisplayableDay *>(cmpo_p.get());
	if (day_p == nullptr)
	{
		cout << "error getting day from month" << endl;
		return nullptr;
	}
	if (granularity == "day")
	{
		DisplayableComponent::depth = day;
		return cmpo_p;
	}
	else
		return nullptr;
}

shared_ptr<DisplayableComponent> FullCalendarBuilder::buildDay(std::tm d, std::shared_ptr<DisplayableComponent> p) {
	shared_ptr<DisplayableComponent> day = make_shared<DisplayableDay>(d, p);
	return day;
}

shared_ptr<DisplayableComponent> FullCalendarBuilder::buildMonth(std::tm d, std::shared_ptr<DisplayableComponent> p) {
	int index = d.tm_mon;
	shared_ptr<DisplayableComponent> m = make_shared<DisplayableMonth>(d, p, CalendarComponent::months[index], CalendarComponent::days[index]);
	for (int i = 0; i < CalendarComponent::days[index]; ++i) { // for each day in the month
		m->addComponent(buildDay(d, m)); // construct day and add as a child of the month
		++(d.tm_mday); // increment day of the month
		d.tm_wday = (d.tm_wday + 1) % CalendarComponent::DAYSINAWEEK; // increment weekday, reset to 0 if needed
	}
	return m;
}
shared_ptr<DisplayableComponent> FullCalendarBuilder::buildYear(std::tm d, std::shared_ptr<DisplayableComponent> p) {
	shared_ptr<DisplayableComponent> y = make_shared<DisplayableYear>(d,p,false);
	// construct each month and add it as a child of the year
	for (unsigned int i = 0; i < CalendarComponent::MONTHS; ++i) {
		d.tm_mon = i;
		y->addComponent(buildMonth(d, y));
		// set week day of first day of the next month
		d.tm_wday = (d.tm_wday + CalendarComponent::days[i]) % CalendarComponent::DAYSINAWEEK; 
	}
	return y;
}