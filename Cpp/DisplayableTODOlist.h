#pragma once
#include "CalendarComponent.h"

class FullCalendarBuilder;
class CalendarInterface;

class DisplayableTODOlist : public CalendarComponent {
	friend FullCalendarBuilder;
	friend CalendarInterface;
public:
	static DisplayableTODOlist* todolist(std::tm, std::shared_ptr<DisplayableComponent>);
	virtual void display(int depth) override;
protected:
	virtual std::shared_ptr<DisplayableComponent> addComponent(std::shared_ptr<DisplayableComponent>) override;
private:
	static DisplayableTODOlist* todolist_p;
	DisplayableTODOlist(std::tm, std::shared_ptr<DisplayableComponent>);
};