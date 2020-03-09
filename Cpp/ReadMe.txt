Lab 5 Readme.

Group members:
  Zihao Chen 467835
  Matheus Bustamante 466212

 We did not encounter any warnings or errors when building the program.

 How the work was splitted between the group members:
	Zihao was in charge of creating a class for events, part.1 of the lab;
	implementing the functionality of creating a new event, searching for an
	event by name, and jumping to a specific date, part.2 of the lab; implementing
	the functionality of the TODO list, part.3 of the lab. 
	Matheus was in charge of implementing the save a calendar to a file, restore a
	calendar from a file and the calendar's view method, part.2 of the lab; Implementing
	the update the display of each individual component, part. 2 of the lab. Implementing 
	the merge a file using the decorator design pattern, part.3 of the lab. 


1. Part 2.1: Adding events to the calendar
Think about the separation of concerns in the project. 
What class/classes is responsible for the user interface of the Calendar?
The class responsible for the user interface is the calendarInterface. It is in this class that all the user 
iteraction with the program will happen. All methods implemented for part II of the lab happened in the CalendarInterface. Functionalities such as the save to calendar, restore from a calendar, jump to a date, search for an event, delete 
and edit an event, and zoom in and zoom out. 

What class/classes is responsible for managing the representation and construction of the Calendar?
The classes responsible for managing the construction of the calendar are the 'fullCalendarBuilder' and the 
'calendarBuilder'. The 'fullCalendarBuilder' class iherits from the 'calendarBuilder' and it is responsible for 
adding all the components of the calendar, they are, a new created event, the year, the month, and the day.

Which class should handle collecting information from the user about an event?
The calendarInterface class handles the iteration from the user. This class is responsible for the user iteraction 
and therefore is responsible for gathering all the information given by the user about an event. After getting all the 
event information and verifiying the information is valid, this class will call the buildEvent method located on the
fullCalendarBuilder class where an event component is added to the calendar.

Which class should handle constructing and adding the event to the Calendar?
The FullcalendarBuilder class should handle the addition of an event to the calendar. In this class a method 
named buildEvent is declared. This class is responsible for adding a component with the event information that 
will then be displayed by the classes responsible for displaying the calendar.

2. Part 2.2 Make the calendar interactive
How did you decide to efficiently search for an event by name?
Everytime a new event is created or merged from a file, the event is inserted into a multimap. The key of the multimap 
is the event name, and the value a shared pointer of the DisplayableComponent type. The method first asks the user to 
input the name of the event the user is looking for. After receiving this information from the user, the method iterates thought the multimap and adds to a vector all the events with the same name as the one inputted by the user. With the 
vector containing all the events the user is looking for, the method sorts the events and display each of them on the terminal.


Which object in the project understands the structure of the Calendar? Which object should be responsible for jumping to a specific object in the Calendar?
An object that was implemented called GetComponentByDate. This object is called in the jump method after the user 
specifies the date, month, and year the calendar should display, as well as the granularity which represents the 
view of the calendar. That could be either a day, month, or year view.


How did you design saving/restoring the calendar to/from a file? What information needs to be saved about a Calendar to successfully restore it?
Saving the calendar: 
This method will first push_back all the events in the calendar to a vector of type shared_ptr<DisplayableComponent> 
where the information for each event can be accessed. After that it will get the calendar name from the first event and 
write it to the first line of the file. And for every other event in the vector, it will store the event name, day, month, year, hour, minute, second, and the number one for both the recurrfor and recurrevery. We did that because the method saves all the events to the file without worrying about the recurrences. Here is an example of how the file looks like after saving a calendar:

Calendar1									              (Calendar Name)
event1 12 12 2019 12 12 12 1 1   -->    (Event Name) (Date) (Month) (Year) (Hour) (Minutes) (Seconds) (RecurEvery) (RecurFor)
event1 1 12 2020 9 12 12 1 1				    (Event Name) (Date) (Month) (Year) (Hour) (Minutes) (Seconds) (RecurEvery) (RecurFor)
event2 12 9 2019 12 12 12 1 1				    (Event Name) (Date) (Month) (Year) (Hour) (Minutes) (Seconds) (RecurEvery) (RecurFor)

Restoring a calendar:
This methid will read a file in the format described above. It will then build a new calendar to where all the new events are posted. This is done in order to  overwrite the current calendar. After that, it saves each information to its corresponding variable. The data store on those variables are used when calling the method responsible for building each event. The method called will then build each event in the file to a the newly created calendar. All the events created are then added to a multimap that is used in other methods such as the one responsible for searching an event. At the end, this method updates the current display to display the new calendar with all the restored events. 

To successfully restore a calendar, the calendar name, the event name, the day, the month, and the year as well as the time of the event are important to restore the calendar successfully. In case the file is not on this format, an error message will display explaning why the process failed to happen.


3. Part 2.3 Updating the display
How a particular component in the calendar should be displayed depends on what the current display of the calendar is.
How did you implement this functionality? 
In the calendarInterface header file, a static member variable of type int was declared. This member variable is responsible of holding a number that represents what the current display of the calenadar is. In the source file, it is first initialized to zero, where zero represents the first view of the calendar or the calendar itself. When the user zooms in, this member variable is incremented by one. So if the user zooms in one time, the member variable would now contain a one, wich represents the year view of the calendar. The same, but opposite would happen if the user zoomed out. The value of the member variable would be
subtracted by one. So if the user was on the year view and decided to zoom out to the calendar view, the variable would go from containing a 1 to containing a 0. Now since the variable was declared in the calendar interface header file and the displayable component is a friend class, this variable that was called 'depth' could be used in each of the displable classes. Since that was the case, we decided to change the number of parameters the display function would take, going from not taking any, to taking one pareameter that would represent what the current display of the calendar is. That way, in each of the display methods, we could use 'if statements' to tell the method what to display depending on the current view of the calendar.

Did you make any changes to the interfaces provided in the starter code? If so, how easy was it to 
refactor the code to match this change? 
We did make changes. We decied that would be better to change how the display method was currently working to instead of not taking no parameters, take one that would represent the current view of the calendar. We had to change it for all the display methods that inherted from the displayableComponent class. Those were the DisplayableYear, DisplayableMonth, DisplayableDay, and DisplayableEvent.

How does your implementation maintain flexibility and extensibility of the project? Compare this with other designs you considered.
Our implementation uses a new implemented parameter that represents the current calendar view by overwriting different display methods for each calendar view. We did not consider using any other design pattern.




Part 3 (only answer the questions pertaining to the functionality you chose to implement)
3.1: Merging calendars
Describe the benefits of the Decorator pattern? If you used the decorator pattern, how was it applied to 
support merging calendars?
One of the main benefits of the decorator pattern, in this particular use, is the possibility to update the display method 
at run time in order to display the name of the calendar. But in general the decorator design pattern is very flexibe 
since it adds responsibility at run time, it extends the functionality of a particular object without adding anything 
to that particular object itself, and allows us to use any number of decorators to extend the functionality of a class. 
In our  calendar it is very useful to add the name of the calendar being merged at run time for each particular event. 


If you chose not to use the decorator pattern, how does your design compare with it when 
considering flexibility and extensibility of the design? 
We used the decorator pattern.


3.2 ToDo List
Describe your implementation? How does it support easy extensibility of the project in the future?
How is the singleton pattern used to enforce only a sigle TODO list instance?
The constructor for a todo list is private, so that can not be accessed outside the class. When the static function was called
the member pointer of the class will be dynamically allocated if it is null before. This function is only called once throughout
the entire program, and thus, only one instance of TODO list is created.



PROGRAM BEHAVIOR FOR EACH FUNCTIONALITY

1. Adding an event to the calendar:
	Our program will ask you to first give the event a name (The name of the event should have no space), it will then prompt the user to 
	imform the date of the event. If any of the date fields is invalid, the program will ask the user to retype a valid date. Th same happens with
	the time. If the time given is invalid, the program will ask the user to reenter the time. 

2. Searching for an event:
	The program will ask the user to write the name of the event the user is looking for. After that, if there are more than one event on the calendar with
	the same name, the program will list all the events and ask the user to select one based on its index. When an event is selected, it will display that
	event on the event view of the calendar.

3. Jump to a date:
	The program will ask the user to type a date and a granularity for which the use would like to see the date. If the date does not exists on the calendar,
	it will inform the user that the given date does not exists and ask the user to retype the date again.

4. Save the calendar to a file:
	The program will ask the user to type the name of the file the program would export the file to. The user should input the file name followed by '.txt'.
	Once the file is given, the program will save all the events and the calendar name in the format above.

5. Restore the calendar from a file:
	The program will ask the user to type the file name foloowed by '.txt'. Once the user types the file name, the program reads the file and adds it to a new calendar. 
	If the user is a view different than the calnedar view, it would redirect the user to the calendar view and all the events should be on the calendar when zooming in.

6. Edit and delete:
	The program will prompt the user a message asking if it should delete or edit the file. When it deletes the event, the user needs to zoom out of the event view to see that
	the event was indeed deleted. To edit an event, the user will be promped to type the new information about the event. When all the information is given, it would add the edited
	event to the calendar.

7. Merge a calendar:
	The program will prompt the user a message asking it to inform the name of the file the calendar to be merged is located.The user should input the file name folowed by '.txt'.
	When a calendar is merged, the name of the calendar apppears in front of each event that the merged calendar contains.
	
8. TODO imlementation:
	The program will store the original depth and diplay to a variable and switch to the todolist view. When the user is under todolist view, the program will prompt two options:
	adding a new task and complete an existing task. The user can also choose to quit the todolist, in which the program will switch to view before the user try to access todolist.
