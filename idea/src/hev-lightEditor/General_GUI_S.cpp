#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <string>

#include <iris.h>

#include "General_GUI_S.h"
#include "LightSource_GUI_S.h"
#include "LightModel_GUI_S.h"
#include "Global_GUI_S.h"
#include "File_GUI_S.h"


int CurrentLightNum = 0;
LightModelDesc LightModel;
LightSourceDesc LightSource[8];

Fl_Window *MainWindow;

LightSourceWidgetsDesc LightSourceWidgets;
LightModelWidgetsDesc LightModelWidgets;

Fl_Group *PositionEditGroup;
Fl_Group *ColorEditGroup;

using namespace std ;



////////////////////////////////////////////////////////////////////////

inline void
normalize3 (double v[3])
	{
	double len = sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
	}  // end of normalize3

inline
double vecLen3 (double v[3])
	{
	return sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}  // end of vecLen3


inline double
dot (double a[3], double b[3])
	{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
	}  // end of dot


void hpToXYZ (double hp[2], double xyz[3])
        {
        double radHP[2];

        radHP[0] = (M_PI/180.0) * hp[0];
        radHP[1] = (M_PI/180.0) * hp[1];


        xyz[2] = sin (radHP[1]);
        double cosphi = cos (radHP[1]);

        xyz[0] = cos (radHP[0]) * cosphi;
        xyz[1] = sin (radHP[0]) * cosphi;
        }  // end of hpToXYZ


void xyzToHP (double xyz[3], double hp[2])
        {
        double len = sqrt ( xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2] );

        double unit[3];
        unit[0] = xyz[0]/len;
        unit[1] = xyz[1]/len;
        unit[2] = xyz[2]/len;

        hp[0] = (180.0/M_PI) * atan2 (unit[1], unit[0]);

        hp[1] = (180.0/M_PI) * asin (unit[2]);
        }  // end of xyzToHP


void formatDouble (double d, int precision, char *result)
        {

        int factor = 1;

        for (int i = 0; i < precision; i++)
                {
                factor *= 10;
                }

        sprintf (result, "%g", ((int)(d*factor))/((double)factor));
        }  // end of formatDouble


////////////////////////////////////////////////////////////////////////

//
// convert a C string to a vector of tokens.
//
void
cStrToVec (char *cStr, std::vector<std::string> &vec)
        {
        static std::string line;

        line = cStr;;

        // chop off trailing spaces
        line = line.substr(0,line.find_last_not_of(" ")+1) ;

        // create vector of tokens
        vec = iris::ParseString(line) ;
        }  // end of cStrToVec









////////////////////////////////////////////////////////////////////////

Rel_Pos_Window::Rel_Pos_Window
  (int x, int y, int w, int h, const char *title) :
                                        Fl_Window (w, h, title)
                                        // Fl_Window (x, y, w, h, title)
	{
	end ();
	relativePosition[0] = x;
	relativePosition[1] = y;
	}  // end of Rel_Pos_Window constructor

Msg_Window::Msg_Window
  (int x, int y, int w, int h, const char *title) :
                                        Rel_Pos_Window (x, y, w, h, title)
	{
	end ();
	set_modal ();

	msgLabel = new Fl_Box (1, 1, w, 50);
	if (title != NULL)
		{
		msgLabel->label (title);
		}
	else
		{
		msgLabel->label ("");
		}

	doneButton = new Fl_Button ((w/2)-50, h-50, 100, 30);
	doneButton->label ("OK");
	doneButton->callback (Msg_Window::cb, (void *)this);
	add (msgLabel);
	add (doneButton);
	}  // end of Msg_Window constructor

void
Msg_Window::cb (Fl_Widget *w, void *v)
	{
	((Msg_Window *)v)->hide();
	}  // end of Msg_Window::cb

void
Msg_Window::setMsg (char *msg)
	{
	msgLabel->label (msg);
	}  // end of Msg_Window::setMsg (char *msg)

void 
Rel_Pos_Window::getRelPos (int relPos[2])
// XYZ_Edit_Window::getRelPos (int relPos[2])
	{
	relPos[0] = relativePosition[0];
	relPos[1] = relativePosition[1];
	}  // end of XYZ_Edit_Window


Fl_Adjuster * XYZ_Edit_Window::adjuster (int i)
	{

	if ((i < 0) || (i > 2))
		{
		return NULL;
		}

	return xyzAdj[i];
	}  // end of XYZ_Edit_Window::adjuster

void HP_Edit_Window::hpCB (Fl_Widget *w, long id)
	{
	static char lab[3][100];

	Fl_Positioner *pos = (Fl_Positioner *)w;
	float x, y;

	x = pos->xvalue ();
	y = pos->yvalue ();
	// sprintf (lab[id], "H: %g     P: %g", x, y);
	sprintf (lab[id], "H: %.2f     P: %.2f", x, y);
	pos->label (lab[id]);

	}  // end of hpCB

HP_Edit_Window::HP_Edit_Window 
  (int x, int y, int w, int h, const char *title) :
                                        Rel_Pos_Window (x, y, w, h, title)
	{
	end ();
	set_modal ();


	positioner = new Fl_Positioner (4, 4, w-8, h-28, "Test Label");

	add (positioner);

	positioner->xbounds (-180.0, 180.0);
	positioner->ybounds (90.0, -90.0);
	positioner->xstep (0.5);
	positioner->ystep (0.5);
	positioner->callback (hpCB, 0);

	}  // end of HP_Edit_Window constructor

void XYZ_Edit_Window::adjCB (Fl_Widget *w, long id)
	{
	// printf ("adjuster %d   val = %f\n", id, ((Fl_Adjuster *)w)->value());
	XYZ_Edit_Window *win = (XYZ_Edit_Window *) w->parent();

	// printf ("adjuster %d   val = %f %f %f\n", id, 
		// win->xyzAdj[0]->value(),
		// win->xyzAdj[1]->value(),
		// win->xyzAdj[2]->value());

	// sprintf (win->xyzTxtVal[id], "%g", win->xyzAdj[id]->value());
	// sprintf (win->xyzTxtVal[id], "%.5f", win->xyzAdj[id]->value());
	formatDouble ( win->xyzAdj[id]->value(), 4, win->xyzTxtVal[id]);

	win->xyzLabel[id]->damage (FL_DAMAGE_ALL);

	}  // end of XYZ_Edit_Window::adjCB

XYZ_Edit_Window::XYZ_Edit_Window 
  (int x, int y, int w, int h, const char *title) :
                                        Rel_Pos_Window (x, y, w, h, title)
                                        // Fl_Window (x, y, w, h, title)
	{
	end ();



	// relativePosition[0] = x;
	// relativePosition[1] = y;

	// give everything default values

	for (int i = 0; i < 3; i++)
		{
		xyzLabel[i] = NULL;
		xyz[i] = 0.0;
		// externalFI[i] = NULL;
		}
	adjRange[0] = -10.0;
	adjRange[1] =  10.0;
	adjStep = 0.001;


	// Now build the widget
	

	end ();
	set_modal ();
	hide ();

	int height = h;
	int width = w;


	int startX = (width/6) - 10;
	int incX = width/3;

	int startY = y+5;
	static const char *xyz[3] = {"X", "Y", "Z"};



	startY = 3;

	for (int i = 0; i < 3; i++)
		{
		int xx = startX + i*incX;
		Fl_Box *lab = new Fl_Box (xx, startY, 20, 20, xyz[i]);
		add (lab);
		xyzAdj[i] = new Fl_Adjuster (xx, startY+17, 20, height-80);

		xyzAdj[i]->callback (adjCB, i);

		xyzAdj[i]->range (adjRange[0], adjRange[1]);
		xyzAdj[i]->step (adjStep);
		xyzAdj[i]->value (0.0);
		xyzAdj[i]->soft (0);
		xyzAdj[i]->when (FL_WHEN_RELEASE_ALWAYS | FL_WHEN_CHANGED);
		add (xyzAdj[i]);

		strcpy (xyzTxtVal[i], "0.0");
		xyzLabel[i] = new Fl_Box 
		    ( FL_FLAT_BOX, (xx+10)-(incX/2), xyzAdj[i]->y()+xyzAdj[i]->h()+3, 
			incX, 20, xyzTxtVal[i]);
		add (xyzLabel[i]);
		} 

	startY += height-40;

	Fl_Box *box = new Fl_Box (1, startY, width, height - startY,
		"Click and drag on arrow buttons.\n"
		"Bigger arrows give bigger changes.");
	box->labelsize (10);
	box->labelfont (FL_HELVETICA_ITALIC);

	add (box);


	}  // end of XYZ_Edit_Window constructor


Dir_Edit_Window::Dir_Edit_Window 
  (int x, int y, int w, int h, const char *title) :
                                        XYZ_Edit_Window (x, y, w, h, title)
	{
	end ();

	adjRange[0] = -1.0;
	adjRange[1] =  1.0;
	adjStep = 0.0001;
	for (int i = 0; i < 3; i++)
		{
		xyzAdj[i]->range (adjRange[0], adjRange[1]);
		xyzAdj[i]->step (adjStep);
		xyzAdj[i]->soft (1);
		}

	}


//
//
///////////////////////////////////////////////////////////////////////////
//  
// Initialize data structures: both light source data and 
//


// GEN
static void
setLightWidgetsNull ()
	{
	int i, j;

	for (i = 0; i < 3; i++)
		{
		LightSourceWidgets.rgbScrollbar[i] = NULL;
		LightSourceWidgets.rgbColorGroup[i] = NULL;
		LightSourceWidgets.grayColorGroup[i] = NULL;
		LightSourceWidgets.grayInput[i] = NULL;
		for (j = 0; j < 3; j++)
			{
			LightSourceWidgets.rgbInput[i][j] = NULL;
			}
		}

	LightSourceWidgets.grayScrollbar = NULL;
	LightSourceWidgets.linkAmbDiffSpec = NULL;
	LightSourceWidgets.scaleRGB = NULL;  
	LightSourceWidgets.rgbEditMode = NULL;
#if 0
	LightSourceWidgets.grayEditMode = NULL;
#endif



	// now the LightModelWidgets

	LightModelWidgets.colorControl = NULL;
	LightModelWidgets.localViewer = NULL;
	LightModelWidgets.twoSided = NULL;

	LightModelWidgets.rgbEditMode = NULL;
#if 0
	LightModelWidgets.grayEditMode = NULL;
#endif




	LightModelWidgets.scaleRGB = NULL;

	LightModelWidgets.rgbColorGroup = NULL;
	LightModelWidgets.grayColorGroup = NULL;
	for (i = 0; i < 3; i++)
		{
		LightModelWidgets.rgbInput[i] = NULL;
		LightModelWidgets.rgbScrollbar[i] = NULL;
		}

	LightModelWidgets.grayScrollbar = NULL;
	LightModelWidgets.grayInput = NULL;

	}  // end of setLightWidgetsNull;


//
//  End of initializations...
//
/////////////////////////////////////////////////////////////////////
//
// Make the GUI agree with the light data
//

void
reconcileGuiToCurrLightType ()
	{

	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);

// fprintf (stderr, "in reconcile  ltypeMenu = %p\n", LightSourceWidgets.ltypeMenu);
	LightSourceWidgets.ltypeMenu->value (ls->type);

	extern void ltypeCB (Fl_Widget *w, void *v);
	ltypeCB (LightSourceWidgets.ltypeMenu, (void *)(ls->type));

	}  // end of reconcileGuiToCurrLightType


void
reconcileGuiToCurrSpotPos ()
	{
	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);
	
	// Set the XYZ direction widgets
	for (int i = 0; i < 3; i++)
		{
		// set the adjuster
		LightSourceWidgets.spot.xyzPosAdj[i]->
				value (ls->localLocation[i]);
		// set the label
		XYZ_Edit_Window::adjCB(LightSourceWidgets.spot.xyzPosAdj[i], i);
		// set the Fl_Value_Inputs
        	LightSourceWidgets.spot.xyzPosFI[i]->
				value (ls->localLocation[i]);
		}
	}  // end of reconcileGuiToCurrSpotPos

void
reconcileGuiToCurrLocalPos ()
	{
	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);
	
	// Set the XYZ direction widgets
	for (int i = 0; i < 3; i++)
		{
		// set the adjuster
		LightSourceWidgets.local.xyzPosAdj[i]->
				value (ls->localLocation[i]);
		// set the label
		XYZ_Edit_Window::adjCB(LightSourceWidgets.local.xyzPosAdj[i], i);
		// set the Fl_Value_Inputs
        	LightSourceWidgets.local.xyzPosFI[i]->
				value (ls->localLocation[i]);
		}
	}  // end of reconcileGuiToCurrLocalPos


void
reconcileGuiToCurrSpotDir ()
	{
	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);
	
	// Set the XYZ direction widgets
	for (int i = 0; i < 3; i++)
		{
		// set the adjuster


		LightSourceWidgets.spot.xyzDirAdj[i]->value (ls->spotDirection[i]);
		// set the label
		XYZ_Edit_Window::adjCB (LightSourceWidgets.spot.xyzDirAdj[i], i);
		// set the Fl_Value_Inputs
        	LightSourceWidgets.spot.xyzDirFI[i]->
				value (ls->spotDirection[i]);
		}


	// Set the HP direction widgets
	double hp[2];
	xyzToHP (ls->spotDirection, hp);
	hp[0] = 90 - hp[0];
	if (hp[0] > 180)
		{
		hp[0] -= 360;
		}

	LightSourceWidgets.spot.hpDirPos->xvalue(hp[0]);
	LightSourceWidgets.spot.hpDirPos->yvalue(hp[1]);
	HP_Edit_Window::hpCB (LightSourceWidgets.spot.hpDirPos, 0);

	for (int i = 0; i < 2; i++)
		{
        	LightSourceWidgets.spot.hpDirFI[i]->value (hp[i]);
		}

	// Set the spot direction widgets

	// set the menu
	LightSourceWidgets.spot.dirMenu->
                value (LightSource[CurrentLightNum].spotDirectionType);

        // swap in and out the right group

        LightSourceWidgets.spot.xyzDirGrp->hide();
        LightSourceWidgets.spot.hpDirGrp->hide();
        LightSourceWidgets.spot.lookAtDirGrp->hide();

        switch (LightSource[CurrentLightNum].spotDirectionType)
                {
                case 0:
                        LightSourceWidgets.spot.xyzDirGrp->show ();
                        break;

                case 1:
                        LightSourceWidgets.spot.hpDirGrp->show ();
                        break;

                case 2:
                        LightSourceWidgets.spot.lookAtDirGrp->show ();
                        break;
                }



	// Now we consider whether the current direction is 
	// consistent with the lookAt point.  If it is not, then
	// we change the lookAt point to make them consistent.

	// This inconsitency only occurs when we have changed the
	// the light location or direction when we are not in 
	// lookAt mode.  When we are in lookAt mode, then the
	// callbacks that change location and direction ensure that 
	// consistency is maintained.

	double lookAtDir[3];
	lookAtDir[0] = ls->spotLookAtPt[0] - ls->localLocation[0] ;
	lookAtDir[1] = ls->spotLookAtPt[1] - ls->localLocation[1] ;
	lookAtDir[2] = ls->spotLookAtPt[2] - ls->localLocation[2] ;

	double len = vecLen3 (lookAtDir);
	normalize3 (lookAtDir);

	double currDir[3];
	memcpy (currDir, ls->spotDirection, 3*sizeof(double));
	normalize3 (currDir);

	// Here we're going to first check if the XYZ direction
	// corresponds to the current lookAt direction.
	if (dot (currDir, lookAtDir) < cos (M_PI / 1800.0) )
		{
		// If we are closer than a tenth of a degree,
		// we're not going to change the lookAt point.
		// Otherwise we just set the look at point based
		// on the location and direction.
		ls->spotLookAtPt[0] = ls->spotDirection[0]+ls->localLocation[0];
		ls->spotLookAtPt[1] = ls->spotDirection[1]+ls->localLocation[1];
		ls->spotLookAtPt[2] = ls->spotDirection[2]+ls->localLocation[2];
		}



	// Now we set the widgets to match spotLookAtPt
	for (int i = 0; i < 3; i++)
		{
		LightSourceWidgets.spot.lookAtDirAdj[i]->
					value(ls->spotLookAtPt[i]);
		XYZ_Edit_Window::
			adjCB (LightSourceWidgets.spot.lookAtDirAdj[i], i);
		LightSourceWidgets.spot.lookAtDirFI[i]->
					value (ls->spotLookAtPt[i]);;
		}


	} // reconcileGuiToCurrSpotDir



void
reconcileGuiToCurrInfDir ()
	{
	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);
	
	// Set the XYZ direction widgets
	for (int i = 0; i < 3; i++)
		{
		// set the adjuster

		LightSourceWidgets.inf.xyzDirAdj[i]->value (ls->infDirection[i]);
		// set the label
		XYZ_Edit_Window::adjCB (LightSourceWidgets.inf.xyzDirAdj[i], i);
		// set the Fl_Value_Inputs
        	LightSourceWidgets.inf.xyzDirFI[i]->value (ls->infDirection[i]);
		}


	// Set the HP direction widgets
	double hp[2];
	xyzToHP (ls->infDirection, hp);
	hp[0] = hp[0] + 90;
	if (hp[0] > 180)
		{
		hp[0] -= 360;
		}


	LightSourceWidgets.inf.hpDirPos->xvalue(hp[0]);
	LightSourceWidgets.inf.hpDirPos->yvalue(hp[1]);
	HP_Edit_Window::hpCB (LightSourceWidgets.inf.hpDirPos, 0);

	for (int i = 0; i < 2; i++)
		{
        	LightSourceWidgets.inf.hpDirFI[i]->value (hp[i]);
		}



	} // reconcileGuiToCurrInfDir







void
reconcileGuiToCurrSpotExpAng ()
	{
	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);

	LightSourceWidgets.spot.angleFI->value(ls->spotCutoff);
	((Fl_Slider *)LightSourceWidgets.spot.angleSB)->value(ls->spotCutoff);

	LightSourceWidgets.spot.expFI->value(ls->spotExponent);
	((Fl_Slider *)LightSourceWidgets.spot.expSB)->value(ls->spotExponent);

	}  // end of reconcileGuiToSpotExpAng



void 
reconcileGuiToCurrAtten ()
	{

	LightSourceWidgets.attenVI[0]->
		value ( LightSource[CurrentLightNum].atten[0]);

	LightSourceWidgets.attenVI[1]->
		value ( LightSource[CurrentLightNum].atten[1]);

	LightSourceWidgets.attenVI[2]->
		value ( LightSource[CurrentLightNum].atten[2]);

	if (LightSource[CurrentLightNum].type == INFINITE)
		{
		LightSourceWidgets.attenGrp->deactivate();
		}
	else
		{
		LightSourceWidgets.attenGrp->activate();
		}

	}  // end of reconcileGuiToCurrAtten


void
reconcilePosGuiToCurrLS ()
	{

	reconcileGuiToCurrLightType ();

	reconcileGuiToCurrSpotDir ();
	reconcileGuiToCurrSpotPos ();
	reconcileGuiToCurrSpotExpAng ();
	reconcileGuiToCurrInfDir ();
	reconcileGuiToCurrLocalPos ();

	reconcileGuiToCurrAtten ();
	}

// GEN
int reconcileGuiToCurrentLightSource ()
	{
	int i, j;

	LightSourceDesc *ls = &(LightSource[CurrentLightNum]);


	LightSourceWidgets.lnumMenu->value (ls->glLightNum);;
	LightSourceWidgets.onOff->
			value (ls->on);;
	LightSourceWidgets.nodeMenu->
			value (ls->node);;
	LightSourceWidgets.showGlyph->
			value (ls->showGlyph);;

#if 0
	LightSourceWidgets.positionMode->
			value (ls->inPositionMode);
	LightSourceWidgets.colorMode->
			value (! ls->inPositionMode );
#endif


	double sum = 0;
	for (j = 0; j < 3; j++)
		{ 
		sum += ls->color[ls->currEditColor][j];
		}
	sum /= 3;
	((Fl_Valuator *)LightSourceWidgets.grayScrollbar)->value (1000*sum);


	// If we're in gray edit mode, make the rgb all the same
	if (! ls->inRgbMode )
		{
		ls->color[ls->currEditColor][0] =
		ls->color[ls->currEditColor][1] =
		ls->color[ls->currEditColor][2] = sum;
		}

	((Fl_Valuator *)LightSourceWidgets.rgbScrollbar[0])->
			  value (1000*ls->color[ls->currEditColor][0]);
	((Fl_Valuator *)LightSourceWidgets.rgbScrollbar[1])->
			  value (1000*ls->color[ls->currEditColor][1]);
	((Fl_Valuator *)LightSourceWidgets.rgbScrollbar[2])->
			  value (1000*ls->color[ls->currEditColor][2]);



#if 1
	LightSourceWidgets.rgbColorGroup[0]->deactivate();
	LightSourceWidgets.rgbColorGroup[1]->deactivate();
	LightSourceWidgets.rgbColorGroup[2]->deactivate();
#endif
	LightSourceWidgets.rgbColorGroup[ls->currEditColor]->activate();
	LightSourceWidgets.rgbColorGroup[ls->currEditColor]->box (FL_THIN_UP_FRAME);
	
#if 1
	LightSourceWidgets.grayColorGroup[0]->deactivate();
	LightSourceWidgets.grayColorGroup[1]->deactivate();
	LightSourceWidgets.grayColorGroup[2]->deactivate();
#endif
	LightSourceWidgets.grayColorGroup[ls->currEditColor]->activate();
	LightSourceWidgets.grayColorGroup[ls->currEditColor]->box (FL_THIN_UP_FRAME);
	
			




	for (i = 0; i < 3; i++)
	    {
	    char rgbStr[100];
	    double sum = 0;
	    for (j = 0; j < 3; j++)
		{
		sum += LightSource[CurrentLightNum].color[i][j];
		LightSourceWidgets.rgbInput[i][j]->
		  value ( CLAMP01 (LightSource[CurrentLightNum].color[i][j]) );
		}
	    LightSourceWidgets.grayInput[i]->value (CLAMP01 (sum/3));
	    }
	
	LightSourceWidgets.linkAmbDiffSpec->
				value (LightSource[CurrentLightNum].linkAmbDiffSpec);

	LightSourceWidgets.rgbEditMode->
			value (LightSource[CurrentLightNum].inRgbMode);


	if (LightSource[CurrentLightNum].inRgbMode)
		{
		LightSourceWidgets.rgbEditGroup->show();
		LightSourceWidgets.grayEditGroup->hide();
		LightSourceWidgets.scaleRGB->activate();
		}
	else
		{
		LightSourceWidgets.rgbEditGroup->hide();
		LightSourceWidgets.grayEditGroup->show();
		LightSourceWidgets.scaleRGB->deactivate();
		}

// JGH need to make correct panels visible

	reconcilePosGuiToCurrLS ();

	}  // end of reconcileGuiToCurrentLightSource





int
reconcileGuiToLightModel ()
	{
	double sum = 0;
	for (int j = 0; j < 3; j++)
		{ 
		sum += LightModel.ambient [j];
		LightModelWidgets.
		   rgbInput[j]->value ( CLAMP01 (LightModel.ambient[j]));
		}
	sum /= 3;
	LightModelWidgets.grayInput->value (CLAMP01 (sum));
	((Fl_Valuator *)LightModelWidgets.grayScrollbar)->value (1000*sum);


#if 0
	// If we're in gray edit mode, make the rgb all the same
	if ( ! LightModel.inRgbMode )
		{
		LightModel.ambient[0] =
		LightModel.ambient[1] =
		LightModel.ambient[2] = sum;
		}
#endif

	((Fl_Valuator *)LightModelWidgets.rgbScrollbar[0])->
			  value (1000 * LightModel.ambient[0]);
	((Fl_Valuator *)LightModelWidgets.rgbScrollbar[1])->
			  value (1000 * LightModel.ambient[1]);
	((Fl_Valuator *)LightModelWidgets.rgbScrollbar[2])->
			  value (1000 * LightModel.ambient[2]);

	LightModelWidgets.rgbColorGroup->activate();
	LightModelWidgets.grayColorGroup->activate();
	
	LightModelWidgets.rgbEditMode->
			value (LightModel.inRgbMode);



	if (LightModel.inRgbMode)
		{
		LightModelWidgets.rgbEditGroup->show();
		LightModelWidgets.grayEditGroup->hide();
		LightModelWidgets.scaleRGB->activate();
		}
	else
		{
		LightModelWidgets.rgbEditGroup->hide();
		LightModelWidgets.grayEditGroup->show();
		LightModelWidgets.scaleRGB->deactivate();
		}


	LightModelWidgets.twoSided->value (LightModel.twoSided);
	LightModelWidgets.localViewer->value (LightModel.localViewer);
	LightModelWidgets.colorControl->value(LightModel.colorControlSeparate);

	}  // end of reconcileGuiToLightModel

int
reconcileGuiToData ()
	{

	reconcileGuiToLightModel ();

	reconcileGuiToCurrentLightSource();

	return 0;
	}  // end of reconcileGuiToData

//
// End of reconciling of GUI with data
//
//////////////////////////////////////////////////////////////////////////




#define STD_V_LINE_HEIGHT (30);
#define STD_V_LINE_SPACING (35);
#define MIN_V_LINE_HEIGHT (20);
#define MIN_V_LINE_SPACING (25);

// ----------------- Separator ------------------------

// Here are some auxiliary functions to help with separators


#define H_CENTER_LINE_BOX  FL_FREE_BOXTYPE
#define V_CENTER_LINE_BOX  ((Fl_Boxtype)(FL_FREE_BOXTYPE+1))

// GEN
void horizontalBoxCenterLineDraw (int x, int y, int w, int h, Fl_Color c)
        {

        fl_color (FL_BLACK);
        fl_line (x, y+(h/2), x+w-1, y+(h/2));
        }

// GEN
void verticalBoxCenterLineDraw (int x, int y, int w, int h, Fl_Color c)
        {

        fl_color (FL_BLACK);
        fl_line (x+(w/2), y, x+(w/2), y+h-1);
        }


// GEN

// This one should be called before any widget building

void setupCustomBoxTypes ()
        {
        Fl::set_boxtype (H_CENTER_LINE_BOX,
                horizontalBoxCenterLineDraw, 1, 1, 2, 2);
        Fl::set_boxtype (V_CENTER_LINE_BOX,
                verticalBoxCenterLineDraw, 1, 1, 2, 2);
        }















// GEN
void printEventInfo ()
	{
	printf ("event = %d\n", Fl::event ());
	printf ("event_key = %d\n", Fl::event_key ());
	printf ("event_x = %d\n", Fl::event_x ());
	printf ("event_y = %d\n", Fl::event_y ());
	printf ("event_state = %x\n", Fl::event_state ());
	printf ("event_button = %x\n", Fl::event_button ());


	}  // end of printEventInfo











// GEN
int
initGUI ()
	{


	setLightWidgetsNull ();

	setupCustomBoxTypes ();

	// size of text in text box, and label size
	uchar textsize = (uchar)(1);
	Fl_Window *window = 
    		new Fl_Window(400, 400);
	window->end();

	MainWindow = window;

	Fl_Tabs *outerTabs = new Fl_Tabs (2, 2, 396, 396);
	outerTabs->end();
	window->add (outerTabs);
	// this is the background color of the selected tab
	outerTabs->selection_color ( fl_rgb_color (230, 210, 160) );
	outerTabs->selection_color ( fl_rgb_color (184, 191, 167) );


	Fl_Group *model = buildModelPanel (1, 30, 396, 380);
	// this color is the background color of the unselected tab
	model ->selection_color (fl_rgb_color (213, 204, 186));
	model ->selection_color (fl_rgb_color (170, 163, 149));

	Fl_Group *sources = new Fl_Group (0, 30, 396, 380, "Light Sources");
	sources->end();
	// this color is the background color of the unselected tab
	// sources->selection_color (fl_rgb_color (0, 0, 128));
	sources->selection_color (fl_rgb_color (170, 163, 149));


	Fl_Group *global = buildGlobalPanel (1, 30, 396, 380);
	// Fl_Group *files = buildFilesPanel (1, 30, 396, 380);
	Fl_Group *files = buildFileIOGroup (1, 30, 396, 370);

	global->selection_color (fl_rgb_color (170, 163, 149));
	files->selection_color (fl_rgb_color (170, 163, 149));



	outerTabs->add (sources);
	outerTabs->add (model);
	outerTabs->add (global);
	outerTabs->add (files);

#if 0
	sources->color (fl_rgb_color (127, 201, 127));
	model->color (fl_rgb_color (190, 174, 212));
	global->color (fl_rgb_color (253, 192, 134));
	files->color (fl_rgb_color (255, 255, 153));
#endif

  	buildSourcesPanel (sources);


	return 0;
	}  // end of initGUI

int runGUI (int argc, char **argv)
	{
        MainWindow->show(argc, argv);

#if 0
        while (Fl::wait())
                {
                }
#else

        Fl::run ();
#endif
        return 0 ;
	}  // end of runGUI
