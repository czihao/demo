#include "stdafx.h"
#include "CalendarInterface.h"
#include "FullCalendarBuilder.h"
#include "ExternalEventDecorator.h"
#include<iostream>
#include<sstream>
#define DAYVALID(d) ((int)(d) <= 31 && (int)(d) >= 1) ? 1 : 0
#define MONVALID(m) ((int)(m) <= 12 && (int)(m) >= 1) ? 1 : 0
#define YEARVALID(y) ((int)(y) >= 1900) ? 1 : 0
#define HRVALID(h) ((int)(h) <= 24 && (int)(h) >= 0) ? 1 : 0
#define MINVALID(min) ((int)(min) <= 59 && (int)(min) >= 0) ? 1 : 0
#define SECVALID(s) ((int)(s) <= 60 && (int)(s) >= 0) ? 1 : 0
using namespace std;

int DisplayableComponent::depth = 0;

CalendarInterface::CalendarInterface(std::string builderType, std::string calendarName, size_t years) : builder(nullptr),cal(nullptr),currentDisplay(nullptr) {
	if (builderType == "full") {
		builder = make_shared<FullCalendarBuilder>();
		cal = builder->buildCalendar(calendarName, years);
		todolist = builder->buildTODOlist();
		currentDisplay = cal;
	}
}

bool CalendarInterface::YearValid(int tm_year)
{
	size_t min_year = cal->dateInfo.tm_year + CalendarComponent::BASEYEAR;
	size_t max_year = min_year + cal->yearsToHold;
	if (tm_year <= max_year && tm_year >= min_year)
	{
		return true;
	}
	return false;
}

void CalendarInterface::run() {
	// run until the user quits
	while (1) {
		// display the current display of the calendar
		currentDisplay->display(DisplayableComponent::depth);
		// display options to the user and respond to user input accordingly
		if (DisplayableComponent::depth == events)
		{
			cout << endl << "zoom out: out, zoom in: in, create an Event: event, search an event by name: search, edit event: edit, jump to a day: jump, merge two calenders: merge, save the calendar: save, restore the calendar: restore, acess the todolist: todo, quit: q " << endl;
		}
		else {
			cout << endl << "zoom out: out, zoom in: in, create an Event: event, search an event by name: search, jump to a day: jump, merge two calenders: merge, save the calendar: save, restore the calendar: restore, acess the todolist: todo, quit: q " << endl;
		}
		string in;
		cin >> in;
		if (in == "in") {
			if (currentDisplay->children.size() > 0) {
				cout << "index? (0-" << currentDisplay->children.size() - 1 << ")" << endl;
				index = 0;
				cin >> index;
				cout << endl;
				zoomIn(index);
			}
			else {
				cout << "Nothing to zoom in on" << endl;
			}
		}
		else if (in == "out") {
			zoomOut();
		}
		else if (in == "event") {
			addEvent();
		}
		else if (in == "merge") {
			getEventInformation();
		}
		else if (in == "search") {
			searchEvent();
		}
		else if (in == "edit") {
			editEvent(index);
		}
		else if (in == "jump") {
			jump();
		}
		else if (in == "save") {
			saveToFile();
		}
		else if (in == "restore") {
			restoreFromFile();
		}
		else if (in == "todo") {
			accessTodo();
		}
		else if (in == "q") {
			break;
		}
		else {
			cout << "please provide a valid input" << endl;
			continue;
		}
	}
}

void CalendarInterface::zoomIn(unsigned int index) {
	shared_ptr<DisplayableComponent> temp = currentDisplay->getChild(index);
	// if the child exists, update the current display to the child. Otherwise, the current view remains the same
	if (temp != nullptr) {
		currentDisplay = temp;
		DisplayableComponent::depth += 1;
	}
}

void CalendarInterface::zoomOut() {
    // if the current display has a parent, update current display to the parent
	if (currentDisplay->getParent().lock() != nullptr) {
		// create a shared_ptr from a weak_ptr, contributing to the objects reference count
		currentDisplay = currentDisplay->getParent().lock();
		DisplayableComponent::depth -= 1;
	}
}

bool CalendarInterface::addEvent() {
	int recday;
	int recfor;
	tm d;
	string event_name;
	string calendar_name = "Calendar1";
	while (true) {
		cout << endl << "enter the name of the event, or q to quit:" << endl;
		if (cin >> event_name)
		{
			if (event_name == "q") {
				return success;
			}
			while (true) {
				cout << endl << "enter the date of the event (dd/mm/yyyy), or q to quit:" << endl;
				string date;
				cin >> date;
				if (date == "q") {
					return success;
				}
				replace(date.begin(), date.end(), '/', ' ');
				istringstream iss(date);
				if (iss >> d.tm_mday && iss >> d.tm_mon && iss >> d.tm_year)
				{
					if (DAYVALID(d.tm_mday)) {
						if (MONVALID(d.tm_mon)) {
							if (YearValid(d.tm_year)) {
								while (true) {
									cout << endl << "enter the time of the event (hr/min/sec), or q to quit:" << endl;
									string time;
									cin >> time;
									if (time == "q") {
										return success;
									}
									replace(time.begin(), time.end(), '/', ' ');
									istringstream iss(time);
									if (iss >> d.tm_hour && iss >> d.tm_min && iss >> d.tm_sec)
									{
										if (HRVALID(d.tm_hour)) {
											if (MINVALID(d.tm_min)) {
												if (SECVALID(d.tm_sec)) {
													while (true) {
														cout << endl << "enter the frequency of the event recur every __ days, recur __ times (recday/recfor), or q to quit:" << endl;
														string freq;
														cin >> freq;
														if (freq == "q") {
															return success;
														}
														replace(freq.begin(), freq.end(), '/', ' ');
														istringstream iss(freq);
														if (iss >> recday && iss >> recfor)
														{
															shared_ptr<DisplayableComponent> event_sp;
															--d.tm_mon;
															d.tm_year -= CalendarComponent::BASEYEAR;
															event_sp = builder->buildEvent(cal, event_name, d, recday, recfor, calendar_name);
															DisplayableEvent *event_p = dynamic_cast<DisplayableEvent *>(event_sp.get());
															if (event_sp == nullptr || event_p == nullptr)
															{
																cout << "error creating an event, please try again" << endl;
															}
															event_mm = builder->event_mm;
															break;
														}
														else {
															cout << endl << "please enter valid recur day and time" << endl;
															continue;
														}
														break;
													}
												}
												else {
													cout << endl << "please enter a valid time" << endl;
													continue;
												}
												break;
											}
											else {
												cout << endl << "please enter a valid time" << endl;
												continue;
											}
											break;
										}
										else {
											cout << endl << "please enter a valid time" << endl;
											continue;
										}
										break;
									}


									else {
										cout << endl << "please enter a valid time" << endl;
										continue;
									}
									break;
								}
							}
							else {
								cout << endl << "please enter a valid date" << endl;
								continue;
							}
							break;

							}
							else {
								cout << endl << "please enter a valid date" << endl;
								continue;
							}
							break;
						}
						else {
							cout << endl << "please enter a valid date" << endl;
							continue;
						}
						break;

				}
				else {
					cout << endl << "please enter a valid date" << endl;
					continue;
				}
				break;
			}
		}
		else {
			cout << endl << "please enter a valid name" << endl;
			continue;
		}
		break;
	}
	return quit;
}

void CalendarInterface::accessTodo() {
	int originalDepth = DisplayableComponent::depth;
	shared_ptr<DisplayableComponent> originalDisplay = currentDisplay;
	currentDisplay = todolist;
	while (true) {
		currentDisplay->display(originalDepth);
		cout << "add a task to the todolist: add, mark a task as complete: complete, exit the todolist: q" << endl;
		string input;
		cin >> input;
		if (input == "add")
		{
			tm d;
			while (true) {
				cout << endl << "enter the name of the task, or q to quit:" << endl;
				string task_name;
				if (cin >> task_name) {
					if (task_name == "q")
					{
						break;
					}
					while (true) {
						cout << endl << "enter the date of the task (dd/mm/yyyy), or q to quit:" << endl;
						string date;
						cin >> date;
						if (date == "q") {
							break;
						}
						replace(date.begin(), date.end(), '/', ' ');
						istringstream iss(date);
						if (iss >> d.tm_mday && iss >> d.tm_mon && iss >> d.tm_year)
						{
							if (DAYVALID(d.tm_mday)) {
								if (MONVALID(d.tm_mon)) {
									if (YearValid(d.tm_year)) {
										while (true) {
											cout << endl << "enter the time of the task (hr/min/sec), or q to quit:" << endl;
											string time;
											cin >> time;
											if (time == "q") {
												break;
											}
											replace(time.begin(), time.end(), '/', ' ');
											istringstream iss(time);
											if (iss >> d.tm_hour && iss >> d.tm_min && iss >> d.tm_sec)
											{
												if (HRVALID(d.tm_hour)) {
													if (MINVALID(d.tm_min)) {
														if (SECVALID(d.tm_sec)) {
															shared_ptr<DisplayableComponent> task_sp;
															--d.tm_mon;
															d.tm_year -= CalendarComponent::BASEYEAR;
															task_sp = builder->buildTask(d, todolist, task_name, false);
															DisplayableTask *task_p = dynamic_cast<DisplayableTask *>(task_sp.get());
															if (task_sp == nullptr || task_p == nullptr)
															{
																cout << "error adding a new task" << endl;
																continue;
															}
															todolist->addComponent(task_sp);
														}
														else {
															cout << endl << "please enter a valid time" << endl;
															continue;
														}
														break;
													}
													else {
														cout << endl << "please enter a valid time" << endl;
														continue;
													}
													break;
												}
												else {
													cout << endl << "please enter a valid time" << endl;
													continue;
												}
												break;
											}
											else {
												cout << endl << "please enter a valid time" << endl;
												continue;
											}
											break;
										}
									}
									else {
										cout << endl << "please enter a valid date" << endl;
										continue;
									}
									break;
								}
								else {
									cout << endl << "please enter a valid date" << endl;
									continue;
								}
								break;
							}
							else {
								cout << endl << "please enter a valid date" << endl;
								continue;
							}
							break;
						}
						else {
							cout << endl << "please enter a valid date" << endl;
							continue;
						}
						break;
					}
				}
				else {
					cout << endl << "please enter a valid name" << endl;
					continue;
				}
				break;
			}
		}
		else if (input == "complete")
		{
			if (todolist->children.size() <= 0)
			{
				cout << "no task to complete" << endl;
				continue;
			}
			cout << "enter the index of the task you complete" << endl;
			string index_str;
			cin >> index_str;
			istringstream iss(index_str);
			int index;
			if (iss >> index)
			{
				if (index >= todolist->children.size() || index < 0)
				{
					cout << "please provide a valid index" << endl;
				}
				else {
					DisplayableTask *task = dynamic_cast<DisplayableTask *>(todolist->children[index].get());
					if (task == nullptr)
					{
						cout << "error completing a task" << endl;
						continue;
					}
					task->complete_task();
				}
			}
			else {
				cout << "error reading index" << endl;
				continue;
			}
		}
		else if (input == "q")
		{
			break;
		}
		else {
			cout << "please provide a valid input to the todolist" << endl;
			continue;
		}
	}
	DisplayableComponent::depth = originalDepth;
	currentDisplay = originalDisplay;
}

int CalendarInterface::getEventInformation() {
	int recday;
	int recfor;
	tm d;
	string filename;
	ifstream datafile;
	cout << "Please enter the input file name: ";
	cin >> filename;
	datafile.open(filename.c_str());


	if (datafile.fail()) {
		cerr << "Error when opening the file" << endl;
		return file_open_failed;
	}

	string calendar_name;
	string event_name;
	string line;
	int event_date;
	int event_mon;
	int event_year;
	int event_hour;
	int event_min;
	int event_sec;
	int event_recday;
	int event_recfor;

	getline(datafile, calendar_name);

	//Now with only the specifications of the piece, read each of them
	while (getline(datafile, line)) {


		stringstream pieces_input;
		pieces_input << line;

		if (pieces_input >> event_name) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_date) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_mon) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_year) {

		}
		else {
			continue;
		}


		if (pieces_input >> event_hour) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_min) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_sec) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_recday) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_recfor) {

		}
		else {
			continue;
		}
		
		d.tm_mday = event_date;
		d.tm_mon = event_mon;
		d.tm_year = event_year;
		d.tm_hour = event_hour;
		d.tm_min = event_min;
		d.tm_sec = event_sec;
		recday = event_recday;
		recfor = event_recfor;

		if (DAYVALID(d.tm_mday))
		{
			if (MONVALID(d.tm_mon)) {
				if (YearValid(d.tm_year)) {
					if (HRVALID(d.tm_hour))
					{
						if (MINVALID(d.tm_min)) {
							if (SECVALID(d.tm_sec)) {
								shared_ptr<DisplayableComponent> event_sp;
								--d.tm_mon;
								d.tm_year -= CalendarComponent::BASEYEAR;
								event_sp = builder->buildEventMerge(calendar_name, cal, event_name, d, recday, recfor);
								DisplayableEvent *event_p = dynamic_cast<DisplayableEvent *>(event_sp.get());
								if (event_sp == nullptr)
								{
									cout << "error merging calendar" << endl;
									break;
								}
								event_mm = builder->event_mm;
							}
							else {
								cout << "Invalid time in the calendar" << endl;
								continue;
							}
						}
						else {
							cout << "invalid time in the calendar" << endl;
							continue;
						}

					}
					else {
						cout << "Invalid time in the calendar" << endl;
						continue;
					}
				}
				else {
					cout << "Invalid date in the calendar" << endl;
					continue;
				}
			}
			else {
				cout << "Invalid date in the calendar" << endl;
				continue;
			}
		}
		else {
			cout << "Invalid date in the calendar" << endl;
			continue;
		}
	}
	return success;
}

int CalendarInterface::saveToFile() {
	vector< shared_ptr<DisplayableComponent> > event_v;
	string filename;
	ofstream datafile;

	for (std::multimap<std::string, std::shared_ptr<DisplayableComponent>>::iterator it = event_mm.begin(); it != event_mm.end(); ++it) {
		event_v.push_back(it->second);
	}

	if (event_v.empty())
	{
		cout << "No events in the calendar " << endl;
		return no_events;
	}
	else {
		cout << "Please name the file you would like to save the calendar in: ";
		cin >> filename;
		datafile.open(filename);
		for (int i = 0; i < 1; ++i) {
			if (dynamic_cast<ExternalEventDecorator*>(event_v[i].get())) {
				ExternalEventDecorator *event_p = dynamic_cast<ExternalEventDecorator *>(event_v[i].get());
				datafile << event_p->calendar_name << endl;
			}
			else {
				DisplayableEvent *event_p = dynamic_cast<DisplayableEvent *>(event_v[i].get());
				datafile << event_p->calendar_name << endl;
			}
		
		}
		for (int i = 0; i < event_v.size(); ++i) {
			if (dynamic_cast<ExternalEventDecorator*>(event_v[i].get())) {
				ExternalEventDecorator *event_p = dynamic_cast<ExternalEventDecorator *>(event_v[i].get());
				datafile << event_p->event_name << " " << event_p->when.tm_mday << " " << event_p->when.tm_mon + 1 << " " << event_p->when.tm_year + CalendarComponent::BASEYEAR << " " << event_p->when.tm_hour << " " << event_p->when.tm_min << " " << event_p->when.tm_sec << " " << 1 << " " << 1 << endl;
			}
			else {
				DisplayableEvent *event_p = dynamic_cast<DisplayableEvent *>(event_v[i].get());

				datafile << event_p->event_name << " " << event_p->dateInfo.tm_mday  << " " << event_p->dateInfo.tm_mon + 1 << " " << event_p->dateInfo.tm_year + CalendarComponent::BASEYEAR << " " << event_p->dateInfo.tm_hour << " " << event_p->dateInfo.tm_min << " " << event_p->dateInfo.tm_sec << " " << 1 << " " << 1 << endl;
			}
			
		}
	}
	
	datafile.close();
	return success;
}

int CalendarInterface::restoreFromFile() {

	int recday;
	int recfor;
	tm d;
	string filename;
	ifstream datafile;
	cout << "Please enter the input file name: ";
	cin >> filename;
	datafile.open(filename.c_str());


	if (datafile.fail()) {
		cerr << "Error when opening the file" << endl;
		return file_open_failed;
	}

	string calendar_name;
	string event_name;
	string line;
	int event_date;
	int event_mon;
	int event_year;
	int event_hour;
	int event_min;
	int event_sec;
	int event_recday;
	int event_recfor;

	getline(datafile, calendar_name);

	cal = builder->buildCalendar(calendar_name, cal->yearsToHold);

	//Now with only the specifications of the piece, read each of them
	while (getline(datafile, line)) {


		stringstream pieces_input;
		pieces_input << line;

		if (pieces_input >> event_name) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_date) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_mon) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_year) {

		}
		else {
			continue;
		}


		if (pieces_input >> event_hour) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_min) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_sec) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_recday) {

		}
		else {
			continue;
		}

		if (pieces_input >> event_recfor) {

		}
		else {
			continue;
		}

		d.tm_mday = event_date;
		d.tm_mon = event_mon;
		d.tm_year = event_year;
		d.tm_hour = event_hour;
		d.tm_min = event_min;
		d.tm_sec = event_sec;
		recday = event_recday;
		recfor = event_recfor;

		if (DAYVALID(d.tm_mday))
		{
			if (MONVALID(d.tm_mon)) {
				if (YearValid(d.tm_year)) {
					if (HRVALID(d.tm_hour)) {
						if (MINVALID(d.tm_min)) {
							if (SECVALID(d.tm_sec)) {

								shared_ptr<DisplayableComponent> event_sp;
								--d.tm_mon;
								d.tm_year -= CalendarComponent::BASEYEAR;
								event_sp = builder->buildEvent(cal, event_name, d, recday, recfor, calendar_name);

								DisplayableEvent *event_p = dynamic_cast<DisplayableEvent *>(event_sp.get());
								if (event_sp == nullptr || event_p == nullptr)
								{
									cout << "error creating an event, please try again" << endl;
								}
								event_mm.insert(make_pair(event_p->event_name, event_sp));
								event_mm = builder->event_mm;

								currentDisplay = cal;


							}
							else {
								cout << "Invalid time in the calendar" << endl;
								continue;
							}
						}
						else {
							cout << "Invalid time in the calendar" << endl;
							continue;
						}
					}
					else {
						cout << "Invalid time in the calendar" << endl;
						continue;
					}
				}
				else {
					cout << "Invalid date in the calendar" << endl;
					continue;
				}
			}
			else {
				cout << "Invalid date in the calendar" << endl;
				continue;
			}
		}
		else {
			cout << "Invalid date in the calendar" << endl;
			continue;
		}
	}
	DisplayableComponent::depth = calendar;
	return success;
}

void CalendarInterface::searchEvent() {
	string event_name;
	vector< shared_ptr<DisplayableComponent> > event_v;
	while (true) {
		cout << "please enter the name of event, or q to quit:" << endl;
		cin >> event_name;
		if (event_name == "q") {
			return;
		}
		auto range = event_mm.equal_range(event_name);
		for (auto it = range.first; it != range.second; ++it) {
			event_v.push_back(it->second);
		}
		if (event_v.empty())
		{
			cout << "no match found for event " << event_name << endl;
			continue;
		}
		break;
	}
	sort(event_v.begin(), event_v.end(), compare);
	for (int i = 0; i < event_v.size(); ++i) {
		cout << "\t" << i << " ";
		event_v[i]->display(year);
	}

	int eventInterested;
	cout << "Please enter the index of the event you would like to select: ";
	cin >> eventInterested;
	if (eventInterested >  event_v.size()) {
		cout << "Please enter a valid index" << endl << endl;
	}
	else {
		for (int i = 0; i < event_v.size(); ++i) {
			if (eventInterested == i) {
				cout << "\t";
				event_v[i]->display(events);
			}
		}
	}




}

void CalendarInterface::jump() {
	string date;
	string granularity;
	tm to_day;
	while (true) {
		cout << "please enter the date and granularity (dd/mm/yyyy/granularity(year/month/day)), or q to quit:" << endl;
		cin >> date;
		if (date == "q")
		{
			return;
		}
		replace(date.begin(), date.end(), '/', ' ');
		istringstream iss(date);
		if (iss >> to_day.tm_mday && iss >> to_day.tm_mon && iss >> to_day.tm_year
			&& iss >> granularity)
		{
			if (DAYVALID(to_day.tm_mday)) {
				if (MONVALID(to_day.tm_mon)) {
					if (YearValid(to_day.tm_year)) {
						if (granularity == "day" || granularity == "month" || granularity == "year")
						{
							--to_day.tm_mon;
							to_day.tm_year -= CalendarComponent::BASEYEAR;
							currentDisplay = builder->getComponentByDate(cal, to_day, granularity);
							break;
						}
						else {
							cout << "invalid granularity" << endl;
							continue;
						}
					}
					else {
						cout << "invalid date" << endl;
						continue;
					}

				}
				else {
					cout << "invalid date" << endl;
					continue;
				}
			}
			else {
				cout << "invalid date" << endl;
				continue;
			}
		}
		else {
			cout << "This date does not exists in the calendar" << endl;
			continue;
		}
	}
	return;
}

void CalendarInterface::editEvent(int index) {
	string in;
	string event_name;
	shared_ptr<DisplayableComponent> event_parent = currentDisplay->getParent().lock();
	DisplayableEvent *event_p = dynamic_cast<DisplayableEvent *>(currentDisplay.get());
	DisplayableDay *day_p;
	if (event_p == nullptr)
	{
		cout << "unable to edit event" << endl;
		return;
	}
	event_name = event_p->event_name;
	while (true) {
		cout << "enter del to delete this event, or edit to edit this event, or q to quit:" << endl;
		cin >> in;
		if (in == "q")
		{
			return;
		}
		if (in == "del")
		{
			//delete the event from its parent
			day_p = dynamic_cast<DisplayableDay *>(event_parent.get());
			day_p->children.erase(day_p->children.begin() + index);
			typedef multimap<std::string, std::shared_ptr<DisplayableComponent>>::iterator iterator;
			pair<iterator, iterator> itr_pair = event_mm.equal_range(event_name);
			iterator itr = itr_pair.first;
			//find the event in the multimap and delete it
			for (; itr != itr_pair.second; ++itr) {
				if (itr->second == currentDisplay)
				{
					event_mm.erase(itr);
					break;
				}
			}
			break;
		}
		else if (in == "edit")
		{
			//add a new event and delete the old one
			if (addEvent())
			{
				day_p = dynamic_cast<DisplayableDay *>(event_parent.get());
				day_p->children.erase(day_p->children.begin() + index);
				typedef multimap<std::string, std::shared_ptr<DisplayableComponent>>::iterator iterator;
				pair<iterator, iterator> itr_pair = event_mm.equal_range(event_name);
				iterator itr = itr_pair.first;
				//find the event in the multimap and delete it
				for (; itr != itr_pair.second; ++itr) {
					if (itr->second == currentDisplay)
					{
						event_mm.erase(itr);
						break;
					}
				}
				break;
			}
			else {
				cout << "unable to edit event" << endl;
				continue;
			}
		}
		else {
			cout << "please provide a valid input" << endl;
			continue;
		}
	}
}