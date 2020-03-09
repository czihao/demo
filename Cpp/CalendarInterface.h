/*
Author: Jon Shidal
Purpose: declare the user interface for our calendar
*/
#pragma once
#include "CalendarBuilder.h"
#include "CalendarComponent.h"
#include "DisplayableEvent.h"
#include "DisplayableDay.h"
#include "DisplayableTask.h"
#include<memory>
#include<algorithm>
#include<iterator>
#include<map>
#include <fstream>
#include <string>
#include <vector> 
#include <sstream>

//Return values
enum values {
	success,
	quit,
	file_open_failed,
	no_events,
	wrong_number_of_commands,
	failed_when_extracting_dimentions,
	could_not_read_line
};

class CalendarInterface {
	std::shared_ptr<Calendar> cal; // the calendar
	std::shared_ptr<CalendarBuilder> builder; // builder for managing construction of the calendar
	std::shared_ptr<DisplayableComponent> currentDisplay; // which component the user is currently viewing
	std::multimap<std::string, std::shared_ptr<DisplayableComponent>> event_mm;
	std::shared_ptr<DisplayableComponent> todolist;
public:
	tm when;
	int index;
	int input_day;
	int input_month;
	int input_year;
	int input_recurr;
	// constructor
	// arguments: 1 - what type of builder? 2 - how many years to hold? 
	CalendarInterface(std::string builderType, std::string calendarName, size_t years);


	// calendar traversal functions
	bool YearValid(int);
	void zoomIn(unsigned int index); // zoom in on a child of the current_display
	void zoomOut(); // zoom out to the parent of the current_display
	bool addEvent(); // add an event to calender
	void searchEvent(); // search for an event by its name
	void editEvent(int); // grant user to edit/delete event under event view
	void jump(); // grant user to jump to a specific date with user defined granularity
	void run(); // display the current view to the user and prompt for input
	int getEventInformation(); //Parse the file and merge the events to the calendar
	int saveToFile(); //Saves the current calendar into a file
	int restoreFromFile(); //Restores events from a file and rewrites on the calendar
	void accessTodo();
};