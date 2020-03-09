#include "stdafx.h"
#include "DisplayableTODOlist.h"
#include "DisplayableTask.h"
#include<iostream>
#include<iomanip>

using namespace std;

DisplayableTODOlist::DisplayableTODOlist(tm d, shared_ptr<DisplayableComponent> p) : CalendarComponent(d, p) {}

DisplayableTODOlist* DisplayableTODOlist::todolist_p = NULL;

DisplayableTODOlist* DisplayableTODOlist::todolist(tm d, shared_ptr<DisplayableComponent> p) {
	if (!todolist_p) {
		todolist_p = new DisplayableTODOlist(d, p);
	}
	return todolist_p;
}

void DisplayableTODOlist::display(int depth) {
	cout << "Your TODO list:" << endl;
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] != nullptr) {
			cout << "\t";
			cout << i << " ";
			children[i]->display(depth);
		}
	}
}

shared_ptr<DisplayableComponent> DisplayableTODOlist::addComponent(shared_ptr<DisplayableComponent> compo_p) {
	DisplayableTask *task_p = dynamic_cast<DisplayableTask *>(compo_p.get());
	if (task_p == nullptr)
	{
		return nullptr;
	}
	children.push_back(compo_p);
	sort(children.begin(), children.end(), compareTask);
	return compo_p;
}