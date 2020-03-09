/*
Author: Jon Shidal
Purpose: Declaration of DisplayableComponent. DisplayableComponent is the common base class of all displayable objects.
It declares and defines the interface that all objects in the composite share.
*/
#pragma once
#include<memory>
#include<vector>



//Enum for view depth of the user
enum depth_view {
	calendar,
	year,
	month,
	day,
	events
};

//Responsible for the distance displayed on the terminal for each week
enum calendar_view_spacing{
	sunday = 0,
	monday = 13,
	tuesday = 25,
	Wednesday = 37,
	Thursday = 49,
	friday = 61,
	saturday = 73,
	single_digit_day = 13,
	other_days =12
};

enum first_day_month {
	first_day_true,
	first_day_false ,
	week_length = 7
};



// forward declaration, needed for friend declaration
class FullCalendarBuilder;
class CalendarInterface;

// abstract base class representing a composite object
class DisplayableComponent {
	friend FullCalendarBuilder;
	friend CalendarInterface;
	static int depth; //In relation to the current user's scope (Calendar/year/month/day)
	friend bool compare(std::shared_ptr<DisplayableComponent>, std::shared_ptr<DisplayableComponent>);
	friend bool compareTask(std::shared_ptr<DisplayableComponent>, std::shared_ptr<DisplayableComponent>);
public:
	// defined by concrete classes deriving from DisplayableComponen
	virtual void display(int depth) = 0;
protected:
	// member variables
	std::vector<std::shared_ptr<DisplayableComponent> > children;
	std::weak_ptr<DisplayableComponent> parent; // weak_ptr to avoid cycles, does not contribute to reference count

	// construction, destruction
	DisplayableComponent(std::shared_ptr<DisplayableComponent>);
	virtual ~DisplayableComponent();

	// composite and traversal functions
	virtual std::weak_ptr<DisplayableComponent> getParent();
	virtual std::shared_ptr<DisplayableComponent> getChild(unsigned int);
	virtual std::shared_ptr<DisplayableComponent> addComponent(std::shared_ptr<DisplayableComponent>);
	virtual std::shared_ptr<DisplayableComponent> removeComponent(unsigned int);
};


