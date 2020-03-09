#include "stdafx.h"
#include "DisplayableMonth.h"
#include "DisplayableDay.h"
#include "DisplayableEvent.h"
#include "global.h"
#include <iostream>
#include <iomanip>
using namespace std;
int first_day; //Declared on global.h
int count_day; //Declared on global.h


DisplayableMonth::DisplayableMonth(std::tm d, std::shared_ptr<DisplayableComponent> p, string monthname, unsigned int numdays) : CalendarComponent(d, p), name(monthname), numberOfDays(numdays) {
	// initialize children vector, one spot for each day
	for (size_t i = 0; i < numberOfDays; ++i) {

		children.push_back(nullptr);
	}
}

void DisplayableMonth::display(int depth){
	if (depth == calendar) {}
	else if (depth == year) {
		cout << name << endl;
		for (size_t i = 0; i < children.size(); ++i) {
			if (children[i] != nullptr) { // forward request to all children
				children[i]->display(depth);
			}
		}
	}
	else if (depth == month) {
		cout << endl << endl << name << ":" << endl;
		cout << "Sunday" << setw(12) << "Monday" << setw(13) << "Tuesday" << setw(15) << "Wendnesday" << setw(10) << "Thrusday" << setw(10) << "Friday" << setw(14) << "Saturday" << endl << endl;
		first_day = 0;
		count_day = 0;
		for (size_t i = 0; i < children.size(); ++i) {
			if (children[i] != nullptr) { // forward request to all children
				if (first_day == first_day_true) {
					if (count_day == 0) {
						children[i]->display(depth);
						count_day++;
						first_day++;
					}
				}
				else {
					if (count_day < week_length) {
						children[i]->display(depth);
						count_day++;
					}
					else {
						cout << endl;
						children[i]->display(depth);
						count_day = first_day_false;
					}
				}
			}
		}
	}
	else if(depth == day){
		for (size_t i = 0; i < children.size(); ++i) {
			cout << "\t";
			cout << endl << endl << name << ":" << endl;
			if (children[i] != nullptr) { // forward request to all children
				children[i]->display(depth);
			}
		}
	}
	else if (depth == events) {
		for (size_t i = 0; i < children.size(); ++i) {
			cout << "\t";
			if (children[i] != nullptr) { // forward request to all children
				children[i]->display(depth);
			}
		}
	}

}

shared_ptr<DisplayableComponent> DisplayableMonth::addComponent(shared_ptr<DisplayableComponent> comp) {
	// try to dynamically cast comp to a pointer to a DisplayableDay, will fail if the DisplayableComponent is not a day
	DisplayableDay* d = dynamic_cast<DisplayableDay *>(comp.get()); // can only cast regular pointers, not shared_ptrs

	if (d == nullptr) { // the cast failed
		return nullptr;
	}

	// otherwise, add the day to the correct location
	int dayOfMonth = d->dateInfo.tm_mday - 1;
	if (children[dayOfMonth] == nullptr) { // day does not already exist
		children[dayOfMonth] = comp;
		return comp;
	}
	else {  // day already exist, return existing day
		return children[dayOfMonth];
	}
}
