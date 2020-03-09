/*
Author: Jon Shidal
Purpose: create a CalendarInterface object and call run
*/

#include "stdafx.h"
#include "CalendarInterface.h"

using namespace std;

int main(int argc, char * argv[]) {
	CalendarInterface ci("full", "test", 3);
	ci.run();
	return 0;
}