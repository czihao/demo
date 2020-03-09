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

DisplayableDay::DisplayableDay(std::tm d, std::shared_ptr<DisplayableComponent> p) : CalendarComponent(d, p), events_in_day(0) { }

void DisplayableDay::display(int depth) {


	if (depth == calendar) {}
	else if (depth == year) {
		//Loop over each event when implemented
		for (size_t i = 0; i < children.size(); ++i) {
			if (children[i] != nullptr) {
				cout << "\t\t";
				children[i]->display(depth);
			}
		}	
	}
	else if(depth == month) {
		if (first_day == first_day_true) { //it is the first day of the month
			if (daysoftheweek[dateInfo.tm_wday] == "Sunday") {
				cout << dateInfo.tm_mday;
			}
			else if (daysoftheweek[dateInfo.tm_wday] == "Monday") {
				count_day++;
				cout << setw(monday) << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
			else if (daysoftheweek[dateInfo.tm_wday] == "Tuesday") {
				count_day = count_day + 2;
				cout << setw(tuesday) << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
			else if (daysoftheweek[dateInfo.tm_wday] == "Wednesday") {
				count_day = count_day + 3;
				cout << setw(Wednesday) << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
			else if (daysoftheweek[dateInfo.tm_wday] == "Thursday") {
				count_day = count_day + 4;
				cout << setw(Thursday) << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
			else if (daysoftheweek[dateInfo.tm_wday] == "Friday") {
				count_day = count_day + 5;
				cout << setw(friday) << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
			else if (daysoftheweek[dateInfo.tm_wday] == "Saturday") {
				count_day = count_day + 6;
				cout << setw(saturday) << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
		}
		else if (first_day == first_day_false) {
			if (daysoftheweek[dateInfo.tm_wday] == "Sunday") {
				cout << endl;
				cout << dateInfo.tm_mday;
				for (size_t i = 0; i < children.size(); ++i) {
					if (children[i] != nullptr) {
						children[i]->display(depth);
					}
				}
			}
			else {
				if (dateInfo.tm_mday == 10) {
					cout << setw(single_digit_day) << dateInfo.tm_mday;
					for (size_t i = 0; i < children.size(); ++i) {
						if (children[i] != nullptr) {
							children[i]->display(depth);
						}
					}
				}
				else {
					cout << setw(other_days) << dateInfo.tm_mday;
					for (size_t i = 0; i < children.size(); ++i) {
						if (children[i] != nullptr) {
							children[i]->display(depth);
						}
					}
				}
			}
		}
	}
	else if(depth == day) {
		cout << "\t";
		cout << daysoftheweek[dateInfo.tm_wday] << " ";
		cout << dateInfo.tm_mon + 1 << "/" << dateInfo.tm_mday << "/" << dateInfo.tm_year + CalendarComponent::BASEYEAR << endl;
		Display_event(depth);
	}
	else if (depth == events) {
		cout << "\t";
		cout << daysoftheweek[dateInfo.tm_wday] << " ";
		cout << dateInfo.tm_mon + 1 << "/" << dateInfo.tm_mday << "/" << dateInfo.tm_year + CalendarComponent::BASEYEAR << endl;
		Display_event(depth);
	}

}

//TODO overload the comparator
shared_ptr<DisplayableComponent> DisplayableDay::addComponent(std::shared_ptr<DisplayableComponent> comp) {
	children.push_back(nullptr);
	DisplayableEvent *dis_e = dynamic_cast<DisplayableEvent *>(comp.get());
	ExternalEventDecorator *ex_e = dynamic_cast<ExternalEventDecorator *>(comp.get());
	if (dis_e == nullptr && ex_e == nullptr)
	{
		return nullptr;
	}
	++events_in_day;
	if (children[events_in_day - 1] == nullptr)
	{
		children[events_in_day - 1] = comp;
		sort(children.begin(), children.end(), compare);
		return comp;
	}
	else {
		return children[events_in_day - 1];
	}
	return nullptr;
}

void DisplayableDay::Display_event(int depth) {
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] != nullptr) {
			cout << "\t";
			children[i]->display(depth);
		}
	}
}