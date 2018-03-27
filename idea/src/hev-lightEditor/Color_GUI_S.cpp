


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "Color_GUI_S.h"
#include "General_GUI_S.h"
#include "InitLights.h"
#include "sgeComm.h"

using namespace std ;


////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////
//
// Callbacks
//



#if 0

// COLOR
void lscolorModeCB(Fl_Widget *w, long inRgbMode)
	{


	// printf ("lscolorModeCB %d\n", inRgbMode);

	if ( LightSource[CurrentLightNum].inRgbMode != inRgbMode )
		{

		if (inRgbMode)
			{
			// switch from gray to rgb mode
			// Data should be OK, so I'm not going to do anything

			}
		else
			{
			// switch from rgb to gray mode
			for (int i = 0; i < 3; i++) // loop over amb, diff, spec
				{
				double sum = 0;
				for (int j = 0; j < 3; j++)
					{
					sum += LightSource[CurrentLightNum].
							color[i][j];
					}
				sum /= 3;
				for (int j = 0; j < 3; j++)
					{
					LightSource[CurrentLightNum].
						color[i][j] = sum;
					}
				}

			}

		LightSource[CurrentLightNum].inRgbMode = inRgbMode;
		reconcileGuiToCurrentLightSource ();
		}

	}  // end of lscolorModeCB


#endif



void lscolorMode2CB (Fl_Widget *w)
	{
	int inRgbMode = ((Fl_Light_Button *)w)->value();

	// printf ("lscolorMode2CB %d\n", inRgbMode);

	if ( LightSource[CurrentLightNum].inRgbMode != inRgbMode )
		{

		if (inRgbMode)
			{
			// switch from gray to rgb mode
			// Data should be OK, so I'm not going to do anything

			}
		else
			{
			// switch from rgb to gray mode
			for (int i = 0; i < 3; i++) // loop over amb, diff, spec
				{
				double sum = 0;
				for (int j = 0; j < 3; j++)
					{
					sum += LightSource[CurrentLightNum].
							color[i][j];
					}
				sum /= 3;
				for (int j = 0; j < 3; j++)
					{
					LightSource[CurrentLightNum].
						color[i][j] = sum;
					}
				}

			}

		LightSource[CurrentLightNum].inRgbMode = inRgbMode;
		reconcileGuiToCurrentLightSource ();
		}

	}  // end of lscolorMode2CB





// extern void setLightSourceColorDefaults (LightSourceDesc *ls);
void colorResetCB (Fl_Widget *w)
        {
        setLightSourceColorDefaults (LightSource + CurrentLightNum);
        sgeSendCurrLightAll ();
	reconcileGuiToCurrentLightSource ();
        }  // end of colorResetCB



// COLOR
void linkAmbDiffSpecCB (Fl_Widget *w)
	{
	LightSource[CurrentLightNum].linkAmbDiffSpec =
				((Fl_Light_Button *)w)->value();

	// printf ("linkAmbDiffSpecCB: %d\n",
	           // LightSource[CurrentLightNum].linkAmbDiffSpec);

	}  // end of linkAmbDiffSpecCB


// COLOR
void colorButtonCB (Fl_Widget *w, long id)
	{
	LightSourceDesc *ls = &LightSource[CurrentLightNum];

	if (id != ls->currEditColor)
		{
#if 1
		LightSourceWidgets.
			rgbColorGroup[ls->currEditColor]->deactivate();
		LightSourceWidgets.
			rgbColorGroup[ls->currEditColor]->box(FL_NO_BOX);
		LightSourceWidgets.
			grayColorGroup[ls->currEditColor]->deactivate();
		LightSourceWidgets.
			grayColorGroup[ls->currEditColor]->box(FL_NO_BOX);
#endif
		ls->currEditColor = id;
		LightSourceWidgets.
			rgbColorGroup[ls->currEditColor]->activate();
		LightSourceWidgets.
			rgbColorGroup[ls->currEditColor]->box(FL_THIN_UP_FRAME);
		LightSourceWidgets.
			grayColorGroup[ls->currEditColor]->activate();
		LightSourceWidgets.
			grayColorGroup[ls->currEditColor]->box (FL_THIN_UP_FRAME);
		}

	reconcileGuiToCurrentLightSource ();

	}  // end of colorButtonCB


#if 0
// COLOR
static void
sendColor (LightSourceDesc *ls, int icolor)
	{
	static char *colorName[3] = {"AMBIENT", "DIFFUSE", "SPECULAR"};

	printf ("LIGHT %d %s %g %g %g %g\n",
		CurrentLightNum,
		colorName[icolor],
		MIN (MAX(ls->color[icolor][0],0.0),1.0),
		MIN (MAX(ls->color[icolor][1],0.0),1.0),
		MIN (MAX(ls->color[icolor][2],0.0),1.0),
		MIN (MAX(ls->color[icolor][3],0.0),1.0));
	}
#endif


#define ALL_COLOR_COMPONENTS (-1)
#define ALL_COLORS (-1)

// COLOR
static void setColorCurrLS (int whichColor, int whichComponent, double val)
	{
	char rgbStr[10];

	double clampedVal = CLAMP01(val);
	sprintf (rgbStr, "%5.3f", CLAMP01(val));

	if (whichComponent == ALL_COLOR_COMPONENTS)
		{
		LightSource[CurrentLightNum].color[whichColor][0] = 
		LightSource[CurrentLightNum].color[whichColor][1] = 
		LightSource[CurrentLightNum].color[whichColor][2] = val;

#if 0
		LightSourceWidgets.rgbInput[whichColor][0]->input.value(rgbStr);
		LightSourceWidgets.rgbInput[whichColor][1]->input.value(rgbStr);
		LightSourceWidgets.rgbInput[whichColor][2]->input.value(rgbStr);
		LightSourceWidgets.grayInput[whichColor]->input.value(rgbStr);
#else
		LightSourceWidgets.rgbInput[whichColor][0]->value(clampedVal);
		LightSourceWidgets.rgbInput[whichColor][1]->value(clampedVal);
		LightSourceWidgets.rgbInput[whichColor][2]->value(clampedVal);
		LightSourceWidgets.grayInput[whichColor]->value(clampedVal);
#endif


		}
	else
		{
		LightSource[CurrentLightNum].color[whichColor][whichComponent] =
				 val;
#if 0
		LightSourceWidgets.rgbInput[whichColor][whichComponent]->
				value(rgbStr);
#else
		LightSourceWidgets.rgbInput[whichColor][whichComponent]->
				value(clampedVal);

#endif

		}
	} // end of setColorCurrLS


// COLOR
static void scaleColorCurrLS (
				int whichColor, 
				int whichComponent, 
				double factor,
				double defaultValue )
	{
	int i, j, startCol, endCol, startComp, endComp;
	double minVal;
	char rgbStr[10];
	int doGray = 0;;

	// minVal = (factor == 0) ? 0 : 0.001;
	minVal = 0.0;


	// factor = MAX (factor, 0.001);


	if (whichColor == ALL_COLORS)
		{
		startCol = 0;
		endCol = 2;
		}
	else
		{
		startCol = endCol = whichColor;
		}

	if (whichComponent == ALL_COLOR_COMPONENTS)
		{
		doGray = 1;
		startComp = 0;
		endComp = 2;
		}
	else
		{
		startComp = endComp = whichComponent;
		}


	for (i = startCol; i <= endCol; i++)
		{
		double sum = 0;
		for (j = startComp; j <= endComp; j++)
		    {
		    // if factor is non-neg then use it as a factor
		    if (factor > 0.0)
			{
		    	LightSource[CurrentLightNum].color[i][j] *= factor;
			}
		    else if (factor == 0.0)
			{
			// this is a bit of a kludge
#define COLOR_SCALED_TO_ZERO (0.00000013)
			if (LightSource[CurrentLightNum].color[i][j] != 0)
			    {
		    	    LightSource[CurrentLightNum].color[i][j] = 
							COLOR_SCALED_TO_ZERO;
			    }
			}
		    else
			{
			// if factor is negative, then it's a signal that
			// we were unable to calculate a factor, so we should
			// either leave the color unchanged if it's not
			// the value that signals that it had been scaled to
			// zero, or set it to the supplied defaultValue.
			// We do this because when a color has been scaled
			// to zero, we want to raise it back up when we
			// are scaling up in tandem with another value that
			// has started at zero.  But if it became zero not by
			// scaling, then it should stay at zero.
			// this is  all pretty kludgey.
			if (LightSource[CurrentLightNum].color[i][j] == 
							COLOR_SCALED_TO_ZERO)
				{
		    		LightSource[CurrentLightNum].color[i][j] = 
								defaultValue;
				}

			}

		    sum += LightSource[CurrentLightNum].color[i][j];
		

#if 0
		    LightSource[CurrentLightNum].color[i][j] = 
		    MIN (1.0,
			MAX(LightSource[CurrentLightNum].color[i][j], minVal));
#endif

#if 0
		    sprintf (rgbStr, "%5.3f", 
			CLAMP01 (LightSource[CurrentLightNum].color[i][j]) );
		    
		    LightSourceWidgets.rgbInput[i][j]->value(rgbStr);
#else

		    LightSourceWidgets.rgbInput[i][j]->
		      value (CLAMP01(LightSource[CurrentLightNum].color[i][j]));
#endif
		    }

#if 0
		sprintf (rgbStr, "%5.3f", CLAMP01(sum/3.0));
		LightSourceWidgets.grayInput[i]->value(rgbStr);
#else
		LightSourceWidgets.grayInput[i]->value(CLAMP01(sum/3.0));
#endif
		}

			
	}  // end of scaleColorCurrLS






// COLOR
void floatInputCB_unlinked (Fl_Widget *w, long id)
	{
	int i, j;
	Fl_Value_Input *fltIn = (Fl_Value_Input *) w;
	Fl_Scrollbar *sb;
	double val;


#if 0
	sscanf (fltIn->value(), "%lf", &val);
#else
	val = fltIn->value();
#endif

	// printf ("NEW!!!  floatInputCB : id = %d val = %f\n", id, val);

	i = id / 100;  // which color: amb, diff, spec
	j = id % 100;  // which component, r, g, b

#if 0
printf ("floatInputCB_unlinked  color = %d   component = %d   val = %f\n",
i, j, val);
#endif

	if (j == 3)
		{
		// we're in gray mode 
		setColorCurrLS (i, ALL_COLOR_COMPONENTS, val);
		sb = LightSourceWidgets.grayScrollbar;
		}
	else
		{
		// we're in rgb mode
		setColorCurrLS (i, j, val);
		if ( (LightSource[CurrentLightNum].inRgbMode) &&
		     (LightSource[CurrentLightNum].currEditColor == i) )
			{
			sb = LightSourceWidgets.rgbScrollbar[j];
			}
		else
			{
			sb = NULL;
			}
		}

	// printf ("sb = %p\n", sb);

	if (sb != NULL)
		{
		// printf ("sb value = %ld\n", sb->value());
		if ( (sb->value() / 1000.0) != val)
			{
			((Fl_Valuator *)sb)->value (1000*val);
			}
		}

#if 1
	sgeSendCurrColor (LightSource[CurrentLightNum].currEditColor);

#else
	sendColor (LightSource+CurrentLightNum, 
			LightSource[CurrentLightNum].currEditColor);
#endif
	}  // end of floatInputCB_unlinked




// COLOR
void floatInputCB_linked (Fl_Widget *w, long id)
	{
	int i, j;
	Fl_Value_Input *fltIn = (Fl_Value_Input *) w;
	Fl_Scrollbar *sb;
	double val, oldVal, factor;
	


#if 0
	sscanf (fltIn->value(), "%lf", &val);
#else
	val = fltIn->value();
#endif


	// printf ("NEW!!!  floatInputCB : id = %d val = %f\n", id, val);

	i = id / 100;  // which color: amb, diff, spec
	j = id % 100;  // which component, r, g, b

	// the other colors:
	int i1 = (i+1) % 3;
	int i2 = (i+2) % 3;


	int jj = (j == 3) ? 0 : j;
	oldVal = LightSource[CurrentLightNum].color[i][jj];
	factor = (oldVal == 0) ? -1 : (val / oldVal);





	if (j == 3)
		{
		// we're in gray mode 
		setColorCurrLS (i, ALL_COLOR_COMPONENTS, val);

		scaleColorCurrLS (i1, ALL_COLOR_COMPONENTS, factor, val);
		scaleColorCurrLS (i2, ALL_COLOR_COMPONENTS, factor, val);

		sb = LightSourceWidgets.grayScrollbar;
		}
	else
		{
		// we're in rgb mode
		setColorCurrLS (i, j, val);
		scaleColorCurrLS (i1, j, factor, val);
		scaleColorCurrLS (i2, j, factor, val);
		if ( (LightSource[CurrentLightNum].inRgbMode) &&
		     (LightSource[CurrentLightNum].currEditColor == i) )
			{
			sb = LightSourceWidgets.rgbScrollbar[j];
			}
		else
			{
			sb = NULL;
			}
		}

	// printf ("sb = %p\n", sb);

	if (sb != NULL)
		{
		// printf ("sb value = %ld\n", sb->value());
		if ( (sb->value() / 1000.0) != val)
			{
			((Fl_Valuator *)sb)->value (1000*val);
			}
		}

#if 1
	sgeSendCurrColor (0);
	sgeSendCurrColor (1);
	sgeSendCurrColor (2);
#else
	sendColor (LightSource+CurrentLightNum, 0);
	sendColor (LightSource+CurrentLightNum, 1);
	sendColor (LightSource+CurrentLightNum, 2);
#endif
	}  // end of floatInputCB_linked








// COLOR
void floatInputCB (Fl_Widget *w, long id)
	{
	if (LightSource[CurrentLightNum].linkAmbDiffSpec)
		{
		floatInputCB_linked (w, id);
		}
	else
		{
		floatInputCB_unlinked (w, id);
		}
	}  // end of floatInputCB











// COLOR
void scrollbarCB_unlinked (Fl_Widget *w, long id)
	{
	Fl_Value_Input *fltIn;
	char rgbStr[10];
	double val = (((Fl_Scrollbar *)w)->value ())/1000.0;
	int currCol = LightSource[CurrentLightNum].currEditColor;

	//printf ("scrollbarCB : %d   %d\n", id, ((Fl_Scrollbar *)w)->value ());
	if ( LightSource[CurrentLightNum].inRgbMode ) 
		{
		setColorCurrLS (currCol, id, val);
		}
	else
		{
		setColorCurrLS (currCol, ALL_COLOR_COMPONENTS, val);
		}

	
#if 1
	sgeSendCurrColor (LightSource[CurrentLightNum].currEditColor);
#else
	sendColor (LightSource+CurrentLightNum, 
			LightSource[CurrentLightNum].currEditColor);
#endif
	}  // end of scrollbarCB_unlinked

// COLOR
void scrollbarCB_linked (Fl_Widget *w, long id)
	{

	double val = (((Fl_Scrollbar *)w)->value ())/1000.0;
	int i = LightSource[CurrentLightNum].currEditColor;
	int j = id;
	int jj = (j == 3) ? 0 : j;
	double oldVal = LightSource[CurrentLightNum].color[i][jj];
	double factor = (oldVal == 0) ? -1 : (val / oldVal);

	// the other colors:
	int i1 = (i+1) % 3;
	int i2 = (i+2) % 3;



	if (j == 3)
		{
		// we're in gray mode 
		setColorCurrLS (i, ALL_COLOR_COMPONENTS, val);

		scaleColorCurrLS (i1, ALL_COLOR_COMPONENTS, factor, val);
		scaleColorCurrLS (i2, ALL_COLOR_COMPONENTS, factor, val);
		}
	else
		{
		// we're in rgb mode
		setColorCurrLS (i, j, val);
		scaleColorCurrLS (i1, j, factor, val);
		scaleColorCurrLS (i2, j, factor, val);
		}

#if 1
	sgeSendCurrColor (0);
	sgeSendCurrColor (1);
	sgeSendCurrColor (2);
#else
	sendColor (LightSource+CurrentLightNum, 0);
	sendColor (LightSource+CurrentLightNum, 1);
	sendColor (LightSource+CurrentLightNum, 2);
#endif

	}  // end of scrollbarCB_linked

// COLOR
void scrollbarCB (Fl_Widget *w, long id)
	{
	if (LightSource[CurrentLightNum].linkAmbDiffSpec)
		{
		scrollbarCB_linked (w, id);
		}
	else
		{
		scrollbarCB_unlinked (w, id);
		}
	}  // end of scrollbarCB








// COLOR
void scaleRGBCB (Fl_Widget *w)
	{
	double val, factor, ratio;
	LightSourceDesc *ls = LightSource+CurrentLightNum;
	static double lastVal = 1.0;
	int icolor;

	if (LightSource[CurrentLightNum].linkAmbDiffSpec)
		{
		icolor = ALL_COLORS;
		}
	else
		{
		icolor = LightSource[CurrentLightNum].currEditColor;
		}






	// printf ("scaleRGBCB: value = %f\n", ((Fl_Roller *)w)->value());

	if (Fl::event () == FL_RELEASE)
		{
		((Fl_Roller *)w)->value(1.0);
		lastVal = 1.0;
		return;
		}

	val = ((Fl_Roller *)w)->value();
	ratio = val/lastVal;
	factor = 1.0 - (10*(1.0 - ratio));

	// printf ("( val %f  / lastVal %f )  = ratio %f and    factor = %f\n", 
		// val, lastVal, ratio, factor);

	lastVal = val;

	if (LightSource[CurrentLightNum].linkAmbDiffSpec)
		{
		scaleColorCurrLS (ALL_COLORS, ALL_COLOR_COMPONENTS, 
							factor, 0.001);
#if 1
		sgeSendCurrColor (0);
		sgeSendCurrColor (1);
		sgeSendCurrColor (2);

#else
		sendColor (LightSource+CurrentLightNum, 0);
		sendColor (LightSource+CurrentLightNum, 1);
		sendColor (LightSource+CurrentLightNum, 2);
#endif
		}
	else
		{
		scaleColorCurrLS(
				LightSource[CurrentLightNum].currEditColor,
				ALL_COLOR_COMPONENTS, factor, 0.001);

#if 1
		sgeSendCurrColor (LightSource[CurrentLightNum].currEditColor);

#else
		sendColor 	(
				LightSource+CurrentLightNum, 
				LightSource[CurrentLightNum].currEditColor
				);
#endif

		}


	int j;
	double sum = 0;
	for (j = 0; j < 3; j++)
		{ 
		sum += ls->color[ls->currEditColor][j];
		}
	sum /= 3;
	((Fl_Valuator *)LightSourceWidgets.grayScrollbar)->value (1000*sum);

	((Fl_Valuator *)LightSourceWidgets.rgbScrollbar[0])->
			  value (1000*ls->color[ls->currEditColor][0]);
	((Fl_Valuator *)LightSourceWidgets.rgbScrollbar[1])->
			  value (1000*ls->color[ls->currEditColor][1]);
	((Fl_Valuator *)LightSourceWidgets.rgbScrollbar[2])->
			  value (1000*ls->color[ls->currEditColor][2]);



	}  // end of scaleRGBCB



//
// End of callbacks
//
///////////////////////////////////////////////////////////////////////////
//
// Build the GUI
//






// COLOR
Fl_Group *buildOtherEditGroup (int x, int y, int w, int h)
	{
	int currX, currY;
	

	// printf ("buildOtherEditGroup %d %d %d %d\n", x, y, w, h);

	Fl_Group *rtnGroup = new Fl_Group (x, y, w, h);
	rtnGroup->end();


	currX = x+10;

	currY = y+8;

        Fl_Button *resetButton = 
		new Fl_Button (currX+12, currY, 130, 30, "Reset");
	// resetButton->deactivate();
	resetButton->callback (colorResetCB);
	rtnGroup->add (resetButton);
	

	// Fl_Box *label = new Fl_Box (currX, y+60, 50, 30, "Other Edit");
	// rtnGroup->add (label);

#if 0

#if 0
	/////////////////////
	// group for color vs gray mode
	//
	currY += 35;
	Fl_Group *modeGroup = new Fl_Group (currX, currY, 380-x, y+120);
	modeGroup->end();

	currY += 6;
        Fl_Round_Button *grayMode = 
		new Fl_Round_Button (currX, currY, 380-x, 40, 
				"Edit White Light\nBrightness Only");
	grayMode->type (FL_RADIO_BUTTON);
	grayMode->callback (lscolorModeCB, 0);
	modeGroup->add (grayMode);
	grayMode->set ();

	currY += 42;
        Fl_Round_Button *rgbMode = new Fl_Round_Button 
		(currX, currY, 380-x, 40, "Edit R, G, and B \nIndependently");
	rgbMode->type (FL_RADIO_BUTTON);
	rgbMode->callback (lscolorModeCB, 1);
	modeGroup->add (rgbMode);

	// Fl_Round_Button *rgbEditMode = rgbMode;
	// Fl_Round_Button *grayEditMode = grayMode;

	////////////////

#else

	/////////////////////
	// button for color vs gray mode
	//

	currY += 56;

	Fl_Light_Button *colorEditModeButton = 
		new Fl_Light_Button (currX+12, currY, 130, 60,
		"Edit R, G, and B \nIndependently");
	colorEditModeButton->callback (lscolorMode2CB);

	currY += 24;

	/////////////////////



#endif






	currY += 48;
	Fl_Light_Button *linkAmbDiffSpec = 
		new Fl_Light_Button (currX+12, currY, 130, 60,
		"Link amb, diff,\nspec together");
	linkAmbDiffSpec->callback (linkAmbDiffSpecCB);


#else

	currY += 56;
	Fl_Light_Button *linkAmbDiffSpec = 
		new Fl_Light_Button (currX+12, currY, 130, 60,
		"Link amb, diff,\nspec together");
	linkAmbDiffSpec->callback (linkAmbDiffSpecCB);
	currY += 24;

	currY += 56;
	Fl_Light_Button *colorEditModeButton = 
		new Fl_Light_Button (currX+12, currY, 130, 60,
		"Edit R, G, and B \nIndependently");
	colorEditModeButton->callback (lscolorMode2CB);



#endif






#if 0
	currY += 70;
	Fl_Roller *scaleRGB = new Fl_Roller (currX+60, currY, 20, 380-(y+170), 
						"Scale All");
	scaleRGB->range (2.0, 0.0);
#else
	currY += 74;
	Fl_Roller *scaleRGB = new Fl_Roller (currX, currY, 390-(x+10), 20, 
						"Scale R, G, and B\ntogether");
	scaleRGB->type (FL_HORIZONTAL);
	scaleRGB->range (0.0, 2.0);
#endif

	scaleRGB->callback (scaleRGBCB);
	scaleRGB->step (0.0001);
	// scaleRGB->range (2.0, 0.0);
	scaleRGB->value (1.0);
	scaleRGB->when (FL_WHEN_RELEASE_ALWAYS | FL_WHEN_CHANGED);


	LightSourceWidgets.linkAmbDiffSpec = linkAmbDiffSpec;
	LightSourceWidgets.scaleRGB = scaleRGB;
#if 0
	LightSourceWidgets.rgbEditMode = rgbMode;
	LightSourceWidgets.grayEditMode = grayMode;
#else

	LightSourceWidgets.rgbEditMode = colorEditModeButton;
#endif

	rtnGroup->add (linkAmbDiffSpec);
	rtnGroup->add (scaleRGB);
#if 0
	rtnGroup->add (modeGroup);
#else
	rtnGroup->add (LightSourceWidgets.rgbEditMode);
#endif






	// rtnGroup->hide ();

	return rtnGroup;
	}  // end of buildOtherEditGroup


// COLOR
Fl_Group *buildGrayEditGroup (int x, int y, int w, int h)
	{
	int subGrpX, subGrpY;

	int height = MIN_V_LINE_HEIGHT + 3;
	int spacing = MIN_V_LINE_SPACING + 3;

	Fl_Group *rtnGroup = new Fl_Group (x, y, w, h);
	rtnGroup->end();
	rtnGroup->hide ();


	// Fl_Box *label = new Fl_Box (x+10, y+60, 50, 30, "Gray Edit");
	// rtnGroup->add (label);

        Fl_Box *label = new Fl_Box (x+100, y+6, 80, height, "brightness");
        rtnGroup->add (label);



	subGrpX = x;
	subGrpX = x + 1;
        subGrpY = y;

	// AMBIENT
        subGrpY += spacing;

	Fl_Button *ambButton = 
		new Fl_Button (subGrpX+5, subGrpY, 70, height, "Ambient");
	rtnGroup->add (ambButton);

        // Fl_Group *ambGroup = new Fl_Group (subGrpX, subGrpY, 230, height);
        Fl_Group *ambGroup = new Fl_Group (subGrpX+2, subGrpY-3, 182, height+6);
        ambGroup->end();
	Fl_Value_Input *ambInput =
                new Fl_Value_Input (subGrpX+100, subGrpY, 80, height);
	ambInput->range (0.0, 1.0);
	ambInput->step (0.001);
	ambInput->value (0.0);
	ambGroup->add (ambInput);

	rtnGroup->add (ambGroup);


	// DIFFUSE
        subGrpY += spacing;

	Fl_Button *difButton = 
		new Fl_Button (subGrpX+5, subGrpY, 70, height, "Diffuse");
	rtnGroup->add (difButton);

        // Fl_Group *difGroup = new Fl_Group (subGrpX, subGrpY, 230, height);
        Fl_Group *difGroup = new Fl_Group (subGrpX+2, subGrpY-3, 182, height+6);
        difGroup->end();
	Fl_Value_Input *difInput =
                new Fl_Value_Input (subGrpX+100, subGrpY, 80, height);
	difInput->range (0.0, 1.0);
	difInput->step (0.001);
	difInput->value (0.0);
	difGroup->add (difInput);

	rtnGroup->add (difGroup);


	// SPECULAR
        subGrpY += spacing;

	Fl_Button *specButton = 
		new Fl_Button (subGrpX+5, subGrpY, 70, height, "Specular");
	rtnGroup->add (specButton);

        // Fl_Group *specGroup = new Fl_Group (subGrpX, subGrpY, 230, height);
        Fl_Group *specGroup = new Fl_Group (subGrpX+2, subGrpY-3, 182, height+6);
        specGroup->end();
	Fl_Value_Input *specInput =
                new Fl_Value_Input (subGrpX+100, subGrpY, 80, height);
	specInput->range (0.0, 1.0);
	specInput->step (0.001);
	specInput->value (0.0);
	specGroup->add (specInput);

	rtnGroup->add (specGroup);



	ambButton->callback (colorButtonCB, 0);
	difButton->callback (colorButtonCB, 1);
	specButton->callback (colorButtonCB, 2);

	LightSourceWidgets.grayColorGroup[0] = ambGroup;
	LightSourceWidgets.grayColorGroup[1] = difGroup;
	LightSourceWidgets.grayColorGroup[2] = specGroup;

	LightSourceWidgets.grayInput[0] = ambInput;
	LightSourceWidgets.grayInput[1] = difInput;
	LightSourceWidgets.grayInput[2] = specInput;

	LightSourceWidgets.grayInput[0]->callback (floatInputCB, 003);
	LightSourceWidgets.grayInput[1]->callback (floatInputCB, 103);
	LightSourceWidgets.grayInput[2]->callback (floatInputCB, 203);

	

	// Now the slider:

	subGrpY += spacing + 5;
	height = 390-subGrpY;

// fprintf (stderr, "Color height = %d\n", height);

	Fl_Scrollbar *grayScrollbar = 
		new Fl_Scrollbar (subGrpX+110+20,  subGrpY, 20, height);
	LightSourceWidgets.grayScrollbar = grayScrollbar;
	grayScrollbar->callback (scrollbarCB, 3);

        grayScrollbar->bounds (1000.0, 0.0);
        grayScrollbar->step(10.0);
        grayScrollbar->linesize (1);

	rtnGroup->add (grayScrollbar);


        Fl_Box *zeroLabel =
                new Fl_Box (subGrpX+95, subGrpY+height-43, 30, 40, "0.0");
        Fl_Box *oneLabel =
                new Fl_Box (subGrpX+95, subGrpY+6, 30, 40, "1.0");
        // printf ("Scrollbar subGrpY = %d  height = %d\n", subGrpY, height);

        rtnGroup->add (zeroLabel);
        rtnGroup->add (oneLabel);






	return rtnGroup;
	}  // end of buildGrayEditGroup





// COLOR
Fl_Group *buildRgbEditGroup (int x, int y, int w, int h)
	{
	int subGrpX, subGrpY;
	int height, spacing;
	int i, j;

	height = MIN_V_LINE_HEIGHT + 3;
	spacing = MIN_V_LINE_SPACING + 3;
	// height = STD_V_LINE_HEIGHT;
	// spacing = STD_V_LINE_SPACING;

	Fl_Group *rtnGroup = new Fl_Group (x, y, w, h);
	rtnGroup->end();

	Fl_Box *rLabel = new Fl_Box (x+80, y+6, 47, height, "red");
	rtnGroup->add (rLabel);
	Fl_Box *gLabel = new Fl_Box (x+130, y+6, 47, height, "green");
	rtnGroup->add (gLabel);
	Fl_Box *bLabel = new Fl_Box (x+180, y+6, 47, height, "blue");
	rtnGroup->add (bLabel);


	// subGrpX = x;
	subGrpX = x + 1;
	subGrpY = y+spacing;
	// Fl_Group *ambGroup = new Fl_Group (subGrpX, subGrpY, 230, height);
	Fl_Group *ambGroup = new Fl_Group (subGrpX+2, subGrpY-3, 230-2, height+6);
	ambGroup->end();
	LightSourceWidgets.rgbColorGroup[0] = ambGroup;
	// Fl_Box *ambLabel = new Fl_Box (subGrpX+10, subGrpY, 50, height, "Ambient");
	Fl_Button *ambButton = new Fl_Button (subGrpX+5, subGrpY, 70, height, "Ambient");
	rtnGroup->add (ambButton);
	ambButton->callback (colorButtonCB, 0);
	Fl_Value_Input *ambR = 
		new Fl_Value_Input (subGrpX+80, subGrpY, 47, height);
	ambR->range (0.0, 1.0);
	ambR->step (0.001);
	LightSourceWidgets.rgbInput[0][0] = ambR;
	ambR->value (0.0);
	ambGroup->add (ambR);
	Fl_Value_Input *ambG = 
		new Fl_Value_Input (subGrpX+130, subGrpY, 47, height);
	ambG->range (0.0, 1.0);
	ambG->step (0.001);
	LightSourceWidgets.rgbInput[0][1] = ambG;
	ambG->value (0.1);
	ambGroup->add (ambG);
	Fl_Value_Input *ambB = 
		new Fl_Value_Input (subGrpX+180, subGrpY, 47, height);
	ambB->range (0.0, 1.0);
	ambB->step (0.001);
	LightSourceWidgets.rgbInput[0][2] = ambB;
	ambB->value (0.2);
	ambGroup->add (ambB);
	rtnGroup->add (ambGroup);


	subGrpY += spacing;
	// Fl_Group *difGroup = new Fl_Group (subGrpX, subGrpY, 230, height);
	Fl_Group *difGroup = new Fl_Group (subGrpX+2, subGrpY-3, 230-2, height+6);
	difGroup->end();
	LightSourceWidgets.rgbColorGroup[1] = difGroup;
	// Fl_Box *difLabel = new Fl_Box (subGrpX+10, subGrpY, 50, height, "Diffuse");
	Fl_Button *difButton = new Fl_Button (subGrpX+5, subGrpY, 70, height, "Diffuse");
	rtnGroup->add (difButton);
	difButton->callback (colorButtonCB, 1);
	Fl_Value_Input *difR = 
		new Fl_Value_Input (subGrpX+80, subGrpY, 47, height);
	difR->range (0.0, 1.0);
	difR->step (0.001);
	LightSourceWidgets.rgbInput[1][0] = difR;
	difR->value (0.0);
	difGroup->add (difR);
	Fl_Value_Input *difG = 
		new Fl_Value_Input (subGrpX+130, subGrpY, 47, height);
	difG->range (0.0, 1.0);
	difG->step (0.001);
	LightSourceWidgets.rgbInput[1][1] = difG;
	difG->value (0.1);
	difGroup->add (difG);
	Fl_Value_Input *difB = 
		new Fl_Value_Input (subGrpX+180, subGrpY, 47, height);
	difB->range (0.0, 1.0);
	difB->step (0.001);
	LightSourceWidgets.rgbInput[1][2] = difB;
	difB->value (0.2);
	difGroup->add (difB);
	rtnGroup->add (difGroup);


	subGrpY += spacing;
	// Fl_Group *specGroup = new Fl_Group (subGrpX, subGrpY, 230, height);
	Fl_Group *specGroup = new Fl_Group (subGrpX+2, subGrpY-3, 230-2, height+6);
	specGroup->end ();
	LightSourceWidgets.rgbColorGroup[2] = specGroup;
#if 0
	Fl_Box *specLabel = new Fl_Box (subGrpX+10, subGrpY, 50, height, "Specular");
	specGroup->add (specLabel);
#else
	Fl_Button *specButton = new Fl_Button (subGrpX+5, subGrpY, 70, height, "Specular");
	rtnGroup->add (specButton);
	specButton->callback (colorButtonCB, 2);
#endif
	Fl_Value_Input *specR = 
		new Fl_Value_Input (subGrpX+80, subGrpY, 47, height);
	specR->range (0.0, 1.0);
	specR->step (0.001);
	LightSourceWidgets.rgbInput[2][0] = specR;
	specR->value (0.0);
	specGroup->add (specR);
	Fl_Value_Input *specG = 
		new Fl_Value_Input (subGrpX+130, subGrpY, 47, height);
	specG->range (0.0, 1.0);
	specG->step (0.001);
	LightSourceWidgets.rgbInput[2][1] = specG;
	specG->value (0.1);
	specGroup->add (specG);
	Fl_Value_Input *specB = 
		new Fl_Value_Input (subGrpX+180, subGrpY, 47, height);
	specB->range (0.0, 1.0);
	specB->step (0.001);
	LightSourceWidgets.rgbInput[2][2] = specB;
	specB->value (0.2);
	specGroup->add (specB);
	rtnGroup->add (specGroup);

	for (i = 0; i < 3; i++)
		{
		for (j = 0; j < 3; j++)
			{
			LightSourceWidgets.rgbInput[i][j]->
				callback (floatInputCB, 100*i + j);
			}
		}

	subGrpX = x;
	subGrpY += spacing + 5;

	height = 390-subGrpY;
	
	// printf ("Scrollbar subGrpY = %d  height = %d\n", subGrpY, height);

	Fl_Group *sliderGroup = 
		new Fl_Group (subGrpX, subGrpY, 230, height);
	sliderGroup->end();

	rtnGroup->add (sliderGroup);


#if 0
	Fl_Slider *rSlider = new Fl_Slider (subGrpX+80,  subGrpY, 30, height);
	rSlider->bounds (0.0, 1.0);
	rSlider->step(0.01);
	Fl_Slider *gSlider = new Fl_Slider (subGrpX+130, subGrpY, 30, height);
	gSlider->bounds (0.0, 1.0);
	gSlider->step(0.01);
	Fl_Slider *bSlider = new Fl_Slider (subGrpX+180, subGrpY, 30, height);
	bSlider->bounds (0.0, 1.0);
	bSlider->step(0.01);
#else
	Fl_Scrollbar *rScrollbar = new Fl_Scrollbar (subGrpX+80+15,  subGrpY, 20, height);
	LightSourceWidgets.rgbScrollbar[0] = rScrollbar;
	rScrollbar->bounds (1000.0, 0.0);
	rScrollbar->step(10.0);
	rScrollbar->linesize (1);
	Fl_Scrollbar *gScrollbar = new Fl_Scrollbar (subGrpX+130+15, subGrpY, 20, height);
	LightSourceWidgets.rgbScrollbar[1] = gScrollbar;
	gScrollbar->bounds (1000.0, 0.0);
	gScrollbar->step(10.0);
	gScrollbar->linesize (1);
	Fl_Scrollbar *bScrollbar = new Fl_Scrollbar (subGrpX+180+15, subGrpY, 20, height);
	LightSourceWidgets.rgbScrollbar[2] = bScrollbar;
	bScrollbar->bounds (1000.0, 0.0);
	bScrollbar->step(10.0);
	bScrollbar->linesize (1);

	rScrollbar->callback (scrollbarCB, 0);
	gScrollbar->callback (scrollbarCB, 1);
	bScrollbar->callback (scrollbarCB, 2);


	// Should these also use FL_WHEN_NOT_CHANGED?  I have old code with that
	// rScrollbar->when (FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	// gScrollbar->when (FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	// bScrollbar->when (FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	rScrollbar->when (FL_WHEN_CHANGED);
	gScrollbar->when (FL_WHEN_CHANGED);
	bScrollbar->when (FL_WHEN_CHANGED);

	sliderGroup->add (rScrollbar);
	sliderGroup->add (gScrollbar);
	sliderGroup->add (bScrollbar);

#endif

	Fl_Box *zeroLabel = 
		new Fl_Box (subGrpX+60, subGrpY+height-43, 30, 40, "0.0");
	Fl_Box *oneLabel = 
		new Fl_Box (subGrpX+60, subGrpY+6, 30, 40, "1.0");
	// printf ("Scrollbar subGrpY = %d  height = %d\n", subGrpY, height);

	sliderGroup->add (zeroLabel);
	sliderGroup->add (oneLabel);


















	return rtnGroup;
	}  // buildRgbEditGroup










// COLOR
Fl_Group *buildColorEditGroup (int x, int y, int w, int h)
	{

	Fl_Group *rtnGroup = new Fl_Group (x, y, w, h);
	rtnGroup->end();

	// Fl_Box *label = new Fl_Box (x+10, y+60, 50, 30, "Color Edit");
	// rtnGroup->add (label);

	// rtnGroup->box (FL_BORDER_BOX);
	rtnGroup->box (FL_SHADOW_BOX);


	Fl_Group *rgbBox = buildRgbEditGroup (x, y, 230, h);
	rtnGroup->add (rgbBox);
	LightSourceWidgets.rgbEditGroup = rgbBox;
	
	Fl_Group *grayBox = buildGrayEditGroup (x, y, 230, h);
	rtnGroup->add (grayBox);
	grayBox->hide ();
	LightSourceWidgets.grayEditGroup = grayBox;

	Fl_Group *otherBox = buildOtherEditGroup (231, y, 400, h);
	rtnGroup->add (otherBox);

	return rtnGroup;
	

	}  // end of buildColorEditGroup






//
//
// End of building of the GUI
//
/////////////////////////////////////////////////////////////////////
