#include "stdafx.h"
#include "DisplayableDay.h"
#include "DisplayableEvent.h"
#include "DisplayableComponent.h"
#include "MergeCalendar.h"
#include "ExternalEventDecorator.h"
#include "global.h"
#include <iostream>
#include <iomanip> 

using namespace std;

DisplayableEvent::DisplayableEvent(std::tm d, std::shared_ptr<DisplayableComponent> p, std::string name, std::string calendar, int recurEvery, int recurFor) : CalendarComponent(d, p), event_name(name), calendar_name(calendar), recurEvery(recurEvery), recurFor(recurFor) {}

void DisplayableEvent::display(int depth) {
	if (depth == calendar) {}
	else if (depth == year) {
		cout << dateInfo.tm_mon + 1 << "/" << dateInfo.tm_mday << "/" << dateInfo.tm_year + CalendarComponent::BASEYEAR << " " << dateInfo.tm_hour << ":" << dateInfo.tm_min << " " << event_name << endl;
	}
	else if (depth == month) {
		cout << "*";
	}
	else if (depth == day) {
		cout << dateInfo.tm_hour << ":" << dateInfo.tm_min << " " << event_name << endl;
	}
	else if (depth == events) {
		if (calendar_name == "Calendar1") {
			cout << "\t";
		}
		cout << dateInfo.tm_hour << ":" << dateInfo.tm_min << " " << event_name << endl;
	}
}


