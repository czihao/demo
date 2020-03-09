/*
Author: Jon Shidal
Purpose: Definition of DisplayableComponent class. Defines the implementation that is 
inherited by all concrete classes deriving from it.
*/
#include "stdafx.h"
#include "DisplayableComponent.h"
#include "DisplayableEvent.h"
#include "DisplayableTask.h"
#include "ExternalEventDecorator.h"
#include <iostream>

using namespace std;

DisplayableComponent::DisplayableComponent(shared_ptr<DisplayableComponent> p) : parent(p) {}

bool compare(std::shared_ptr<DisplayableComponent>l_p, std::shared_ptr<DisplayableComponent>r_p) {
	DisplayableEvent *lhs_e = dynamic_cast<DisplayableEvent *>(l_p.get());
	ExternalEventDecorator *lhs_ee = dynamic_cast<ExternalEventDecorator *>(l_p.get());
	DisplayableEvent *rhs_e = dynamic_cast<DisplayableEvent *>(r_p.get());
	ExternalEventDecorator *rhs_ee = dynamic_cast<ExternalEventDecorator *>(r_p.get());
	if (lhs_e != nullptr)
	{
		if (rhs_e != nullptr) {}
		else if (rhs_ee != nullptr)
		{
			rhs_e = dynamic_cast<DisplayableEvent *>(rhs_ee->_component.get());
		}
		else {
			cout << "error comparing two events" << endl;
			return false;
		}
	}
	else if (lhs_ee != nullptr)
	{
		lhs_e = dynamic_cast<DisplayableEvent *>(lhs_ee->_component.get());
		if (rhs_e != nullptr) {}
		else if (rhs_ee != nullptr)
		{
			rhs_e = dynamic_cast<DisplayableEvent *>(rhs_ee->_component.get());
		}
		else {
			cout << "error comparing two events" << endl;
			return false;
		}
	}
	else {
		cout << "error comparing two events" << endl;
		return false;
	}
	int lhs_year = lhs_e->dateInfo.tm_year;
	int rhs_year = rhs_e->dateInfo.tm_year;
	if (lhs_year < rhs_year)
	{
		return true;
	}
	else if (lhs_year == rhs_year) {
		int lhs_month = lhs_e->dateInfo.tm_mon;
		int rhs_month = rhs_e->dateInfo.tm_mon;
		if (lhs_month < rhs_month)
		{
			return true;
		}
		else if (lhs_month == rhs_month) {
			int lhs_day = lhs_e->dateInfo.tm_mday;
			int rhs_day = rhs_e->dateInfo.tm_mday;
			if (lhs_day < rhs_day)
			{
				return true;
			}
			else if (lhs_day == rhs_day)
			{
				int lhs_hr = lhs_e->dateInfo.tm_hour;
				int rhs_hr = rhs_e->dateInfo.tm_hour;
				if (lhs_hr < rhs_hr)
				{
					return true;
				}
				else if (lhs_hr == rhs_hr) {
					int lhs_min = lhs_e->dateInfo.tm_min;
					int rhs_min = rhs_e->dateInfo.tm_min;
					if (lhs_min < rhs_min)
					{
						return true;
					}
					else if (lhs_min == rhs_min)
					{
						int lhs_sec = lhs_e->dateInfo.tm_sec;
						int rhs_sec = rhs_e->dateInfo.tm_sec;
						if (lhs_sec < rhs_sec)
						{
							return true;
						}
						else {
							return false;
						}
					}
					else {
						return false;
					}
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool compareTask(std::shared_ptr<DisplayableComponent>l_t, std::shared_ptr<DisplayableComponent> r_t) {
	DisplayableTask *lhs_e = dynamic_cast<DisplayableTask *>(l_t.get());
	DisplayableTask *rhs_e = dynamic_cast<DisplayableTask *>(r_t.get());
	if (lhs_e == nullptr || rhs_e == nullptr)
	{
		return false;
	}
	int lhs_year = lhs_e->dateInfo.tm_year;
	int rhs_year = rhs_e->dateInfo.tm_year;
	if (lhs_year < rhs_year)
	{
		return true;
	}
	else if (lhs_year == rhs_year) {
		int lhs_month = lhs_e->dateInfo.tm_mon;
		int rhs_month = rhs_e->dateInfo.tm_mon;
		if (lhs_month < rhs_month)
		{
			return true;
		}
		else if (lhs_month == rhs_month) {
			int lhs_day = lhs_e->dateInfo.tm_mday;
			int rhs_day = rhs_e->dateInfo.tm_mday;
			if (lhs_day < rhs_day)
			{
				return true;
			}
			else if (lhs_day == rhs_day)
			{
				int lhs_hr = lhs_e->dateInfo.tm_hour;
				int rhs_hr = rhs_e->dateInfo.tm_hour;
				if (lhs_hr < rhs_hr)
				{
					return true;
				}
				else if (lhs_hr == rhs_hr) {
					int lhs_min = lhs_e->dateInfo.tm_min;
					int rhs_min = rhs_e->dateInfo.tm_min;
					if (lhs_min < rhs_min)
					{
						return true;
					}
					else if (lhs_min == rhs_min)
					{
						int lhs_sec = lhs_e->dateInfo.tm_sec;
						int rhs_sec = rhs_e->dateInfo.tm_sec;
						if (lhs_sec < rhs_sec)
						{
							return true;
						}
						else {
							return false;
						}
					}
					else {
						return false;
					}
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

// zero out all shared_ptr so resources are cleaned up
DisplayableComponent::~DisplayableComponent() {
	for (size_t i = 0; i < children.size(); ++i) {
		// parent is a weak_ptr, so no need to clear children's parent
		children[i] = nullptr;
	}
	children.clear();
}

shared_ptr<DisplayableComponent> DisplayableComponent::addComponent(shared_ptr<DisplayableComponent> comp) {
	// do nothing as default, leaf nodes can't contain components. 
	// Any composite classes that can contain components will override this
	return nullptr;
}

// this method does not shrink the children vector, it simply nulls out the removed index in the vector
shared_ptr<DisplayableComponent> DisplayableComponent::removeComponent(unsigned int index) {
	// go ahead and define a default implementation here, this should be good enough for derived classes(leaf or composite).
	if (index < children.size()) { // leaf objects will have size of 0
		shared_ptr<DisplayableComponent> removed = children[index];
		children[index] = nullptr;
		return removed; // pointer to the removed component if successful
	}
	return nullptr; // nullptr is remove fails
}


weak_ptr<DisplayableComponent> DisplayableComponent::getParent() {
	return parent;
}

shared_ptr<DisplayableComponent> DisplayableComponent::getChild(unsigned int index) {
	// go ahead and define here, leaf classes with no children will return nullptr
	if (index < children.size()) {
		return children[index];
	}
	else {
		return nullptr; // nullptr if index is out of bounds
	}
}



