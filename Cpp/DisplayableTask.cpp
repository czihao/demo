#include "stdafx.h"
#include "DisplayableTask.h"
#include "DisplayableComponent.h"
#include<iostream>
#include<iomanip>

using namespace std;

DisplayableTask::DisplayableTask(tm d, shared_ptr<DisplayableComponent> p, string task_name, bool complete) : CalendarComponent(d, p), task_name(task_name), complete(complete){}

void DisplayableTask::complete_task() {
	complete = true;
}

void DisplayableTask::display(int depth) {
	cout << task_name << " ";
	if (complete) {
		cout << dateInfo.tm_mon + 1 << "/" << dateInfo.tm_mday << "/" << dateInfo.tm_year + CalendarComponent::BASEYEAR;
		cout << " " << dateInfo.tm_hour << ":" << dateInfo.tm_min << "\t" << "COMPLETE" << endl;
	}
	else {
		cout << dateInfo.tm_mon + 1 << "/" << dateInfo.tm_mday << "/" << dateInfo.tm_year + CalendarComponent::BASEYEAR;
		cout << " " << dateInfo.tm_hour << ":" << dateInfo.tm_min << "\t" << "TODO" << endl;
	}
}