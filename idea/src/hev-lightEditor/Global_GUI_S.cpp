#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "Global_GUI_S.h"
#include "General_GUI_S.h"

using namespace std ;





Fl_Group *buildGlobalPanel (int x, int y, int w, int h)
	{
	Fl_Group *globalGroup = new Fl_Group (x, y, w, h, " Global ");
	globalGroup->end();

	// Fl_Box *label = new Fl_Box (x+10, y+60, 250, 60, 
	Fl_Box *label = new Fl_Box (x, y, w, h/2,
			"Global Light Control Panel\nUnder Construction");
	globalGroup->add (label);

	return globalGroup;
	}  // end of buildGlobalPanel


