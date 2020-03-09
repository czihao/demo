#include "stdafx.h"
#include "DisplayableYear.h"
#include <iostream>
#include "DisplayableMonth.h"

using namespace std;

DisplayableYear::DisplayableYear(std::tm d, std::shared_ptr<DisplayableComponent> p, bool l) : CalendarComponent(d, p), leap(l) {
	for (size_t i = 0; i < CalendarComponent::MONTHS; ++i) {
		children.push_back(nullptr);
	}
}

void DisplayableYear::display(int depth) {
	if (depth == calendar) {
		cout <<  dateInfo.tm_year + CalendarComponent::BASEYEAR << endl;
	}
	else if(depth == year) {
		cout << "\t" << "Year " << dateInfo.tm_year + CalendarComponent::BASEYEAR << ":" << endl;
		for (size_t i = 0; i < children.size(); ++i) { // forward request to all children
			cout << "\t" << i << " "; //Index of each month
			if (children[i] != nullptr) {
				children[i]->display(depth);
			}
		}
	}
	else if(depth == month){
		cout << endl << endl << "Year " << dateInfo.tm_year + CalendarComponent::BASEYEAR << ":" << endl;
		for (size_t i = 0; i < children.size(); ++i) { // forward request to all children
			if (children[i] != nullptr) {
				children[i]->display(depth);
			}
		}
	}
	else if(depth == day) {
		cout << endl << endl << "Year " << dateInfo.tm_year + CalendarComponent::BASEYEAR << ":" << endl;
		for (size_t i = 0; i < children.size(); ++i) { // forward request to all children
			cout << "\t";
			if (children[i] != nullptr) {
				children[i]->display(depth);
			}
		}
	}
	else if (depth == events) {
		for (size_t i = 0; i < children.size(); ++i) { // forward request to all children
			cout << "\t";
			if (children[i] != nullptr) {
				children[i]->display(depth);
			}
		}
	}
}

shared_ptr<DisplayableComponent> DisplayableYear::addComponent(shared_ptr<DisplayableComponent> comp) {
	// try to dynamically cast comp to a pointer to a DisplayableMonth
	DisplayableMonth * m = dynamic_cast<DisplayableMonth *>(comp.get());
	if (m == nullptr) { // if the cast fails, return nullptr
		return nullptr;
	}
	// otherwise, add the month to the correct location
	int monthOfYear = m->dateInfo.tm_mon;
	if (children[monthOfYear] == nullptr) { // month does not already exist
		children[monthOfYear] = comp;
		return comp;
	}
	else {  // month already exist, return existing month
		return children[monthOfYear];
	}
}
