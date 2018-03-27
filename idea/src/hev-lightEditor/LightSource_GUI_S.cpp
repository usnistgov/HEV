#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "General_GUI_S.h"
#include "Color_GUI_S.h"
#include "Position_GUI_S.h"
#include "sgeComm.h"

using namespace std ;




///////////////////////////////////////////////////////////////////////////
//
// Callbacks
//

// LS
void lnumCB (Fl_Widget *w, void *v)
	{
	// set current light number
	CurrentLightNum = (long int) (v);

	// printf ("current light source is %d\n", CurrentLightNum);

	reconcileGuiToCurrentLightSource ();
	} // end of lnumCB


static void lsOnOffCB (Fl_Widget *w)
	{

	// printf ("lsOnOffCB %d\n", ((Fl_Light_Button *)w)->value());
	LightSource[CurrentLightNum].on = ((Fl_Light_Button *)w)->value();

	sgeSendCurrOnOff ();

	}  // end of lsOnOffCB;

// LS
void nodeCB (Fl_Widget *w, void *v)
	{

	// 0 no node 1 scene, 2 ether, 3 world, 4 wand, 5 head, 6 nav, 7 unknown

	sgeSendCurrRmFromParentNode ();  // JGH

	// set current node
	LightSource[CurrentLightNum].node = (long int) (v);

	// printf ("current node is %d\n", LightSource[CurrentLightNum].node);

	sgeSendCurrAddToParentNode ();  // JGH


	} // end of lnumCB




// LS
void editModeCB (Fl_Widget *w, long whichButton)
	{


	// printf ("Button %d was selected for widget %p.\n", whichButton, w);
	switch (whichButton)
		{
		case 1:
			PositionEditGroup->hide();
			ColorEditGroup->show();
			LightSourceWidgets.positionMode->box(FL_NO_BOX);
			LightSourceWidgets.colorMode->box(FL_SHADOW_BOX);
			break;

		case 2:
			PositionEditGroup->show();
			ColorEditGroup->hide();
			LightSourceWidgets.positionMode->box(FL_SHADOW_BOX);
			LightSourceWidgets.colorMode->box(FL_NO_BOX);
			break;
		}


	}  // end of editModeCB

//
// End of callbacks
//
///////////////////////////////////////////////////////////////////////////
//
// Build the GUI
//



// LS
void
buildSourcesPanel (Fl_Group *sources)
	{
	int currX, currY;

  	static Fl_Menu_Item lightList[] =
		{
			{"0", 0, lnumCB, (void *)0},
			{"1", 0, lnumCB, (void *)1},
			{"2", 0, lnumCB, (void *)2},
			{"3", 0, lnumCB, (void *)3},
			{"4", 0, lnumCB, (void *)4},
			{"5", 0, lnumCB, (void *)5},
			{"6", 0, lnumCB, (void *)6},
			{"7", 0, lnumCB, (void *)7},
			{NULL}
		};

  	static Fl_Menu_Item nodeList[] =
		{
			{"no node", 0, nodeCB, (void *)0},
			{"scene", 0, nodeCB, (void *)1},
			{"ether", 0, nodeCB, (void *)2},
			{"world", 0, nodeCB, (void *)3},
			{"wand", 0, nodeCB, (void *)4},
			{"head", 0, nodeCB, (void *)5},
			{"nav", 0, nodeCB, (void *)6},
			{"[unknown]", 0, nodeCB, (void *)7, 
						FL_MENU_INACTIVE, 
						FL_NORMAL_LABEL, 
						FL_HELVETICA_ITALIC, 
						FL_NORMAL_SIZE, 
						FL_INACTIVE_COLOR},
			{NULL}
		};



	Fl_Box *lLabel = new Fl_Box (10, 32, 50, 30, "Light ID");
	// lLabel->labelfont (FL_HELVETICA_BOLD_ITALIC);
	lLabel->labelfont (FL_HELVETICA_ITALIC);
	sources->add (lLabel);

	Fl_Choice *lnumMenu = new Fl_Choice (10, 57, 50, 40);
	lnumMenu->menu (lightList);
	lnumMenu->textsize (36);
	lnumMenu->tooltip ("Light ID");
	sources->add (lnumMenu);
	LightSourceWidgets.lnumMenu = lnumMenu;
	

	Fl_Light_Button *onOff = 
		new Fl_Light_Button (75, 40, 100, 30, "on/off");
	sources->add (onOff);
	LightSourceWidgets.onOff = onOff;
	onOff->callback (lsOnOffCB);

	Fl_Choice *nodeMenu = new Fl_Choice (180, 40, 100, 30);
	nodeMenu->menu (nodeList);
	nodeMenu->tooltip ("Load the light under this node.");
	sources->add (nodeMenu);
	LightSourceWidgets.nodeMenu = nodeMenu;

	Fl_Light_Button *showGlyph = 
		new Fl_Light_Button (285, 40, 100, 30, "show glyph");
	showGlyph->deactivate();
	sources->add (showGlyph);
	LightSourceWidgets.showGlyph = showGlyph;

	Fl_Box *sep1 = new Fl_Box (H_CENTER_LINE_BOX, 175, 74, 150, 3, "");
	sources->add (sep1);

	Fl_Box *sep2 = new Fl_Box (V_CENTER_LINE_BOX, 65, 43, 3, 54, "");
	sources->add (sep2);



	///////////////////////////////////////////
	// radio buttons for color vs position panel

	currX = 10;
	currY = 75;
	currY = 85;
	
	Fl_Group *modeGroup = new Fl_Group (currX, currY, 380, 100);

        Fl_Box *modeLabel = new Fl_Box (currX+65, currY, 100, 30, "Edit mode:");
#if 1
	modeLabel->align (FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	modeLabel->labelfont (FL_HELVETICA_ITALIC);
#endif

        Fl_Round_Button *colorMode = 
		new Fl_Round_Button (currX+160, currY, 70, 30, "Color");
	colorMode->type (FL_RADIO_BUTTON);
	colorMode->set ();
	colorMode->callback (editModeCB, 1);
        Fl_Round_Button *positionMode = 
	    new Fl_Round_Button (currX+230, currY, 120, 30, "Position, etc.");
	positionMode->type (FL_RADIO_BUTTON);
	positionMode->callback (editModeCB, 2);
	modeGroup->end();

	LightSourceWidgets.colorMode = colorMode;
	LightSourceWidgets.positionMode = positionMode;

	sources->add (modeGroup);

	LightSourceWidgets.colorMode->box(FL_SHADOW_BOX);
	LightSourceWidgets.positionMode->box(FL_NO_BOX);

	///////////////////////////////////////////




	ColorEditGroup =       buildColorEditGroup (0, 115, 400, 285);
	PositionEditGroup = buildPositionEditGroup (0, 115, 400, 285);
	PositionEditGroup->hide();

	sources->add (ColorEditGroup);
	sources->add (PositionEditGroup);




	}  // end of buildSourcesPanel


// End of building of the GUI
//
/////////////////////////////////////////////////////////////////////
