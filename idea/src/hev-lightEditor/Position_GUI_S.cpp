/*
  Position_GUI_S.cpp
	This code handles the part of the GUI that pertains to
	light type (infinite, omni-directional, spotlight), the
	location, the direction, spot light angle and exponent,
	and the attenuations.

	There is too much code repetition and it is slightly 
	kludgey in places.  If I were to do this over, I'd use
	more widget subclasses and it would be more consistent
	in approach.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <string>

#include "Position_GUI_S.h"
#include "General_GUI_S.h"
#include "InitLights.h"

#include "sgeComm.h"

using namespace std ;


////////////////////////////////////////////////////////////////////////

bool directionsAreClose ( double a[3], double b[3] )
	{
	// I'm going to define "close" as < .01 deg
	double aa[3], bb[3];

	memcpy (aa, a, 3*sizeof(double));
	memcpy (bb, b, 3*sizeof(double));

	normalize3 (aa);
	normalize3 (bb);

	double d = dot (aa, bb);

// printf ("dir are close %f %f\n", d , cos (M_PI/18000.0));

	return ( d > cos (M_PI/18000.0));
	}  // end of vectorsAreClose


void ltypeCB (Fl_Widget *w, void *v)
        {

	int newType;


	LightSourceWidgets.inf.group->hide();
	LightSourceWidgets.local.group->hide();
	LightSourceWidgets.spot.group->hide();
 
	switch (((long int)v))
		{
		case 0:
			LightSourceWidgets.inf.group->show ();
			newType = INFINITE;
			break;

		case 1:
			LightSourceWidgets.local.group->show ();
			newType = OMNI;
			break;

		case 2:
			LightSourceWidgets.spot.group->show ();
			newType = SPOT;
			break;

		}  // end of switch

	if (newType == LightSource[CurrentLightNum].type)
		{
		return;
		}
		
	LightSourceDesc *ls = LightSource + CurrentLightNum;

	if (ls->type == INFINITE)
		{

		for (int i = 0; i < 3; i++)
		    {
		    ls->spotDirection[i] = -ls->infDirection [i];
		    }
			
		// set local location for both spot and omni lights
		if ( ! directionsAreClose ( 
					ls->infDirection, ls->localLocation) )
		    {
		    for (int i = 0; i < 3; i++)
		        {
		        ls->localLocation[i] = ls->infDirection [i]  ;
		        }
		    }

		}
	else if (newType == INFINITE)
		{
		if (ls->type == SPOT)
			{
			for (int i = 0; i < 3; i++)
			        {
				ls->infDirection[i] = -ls->spotDirection [i];
				}
			}
		else if (ls->type == OMNI)
			{
			for (int i = 0; i < 3; i++)
			        {
				ls->infDirection [i]  = ls->localLocation[i];
			        }
			normalize3 (ls->infDirection);
			}
		}

	LightSource[CurrentLightNum].type = newType;;

	// within the next call, the spot lookAt point is made consistent with
	// the spot location and direction
	reconcilePosGuiToCurrLS();

	sgeSendCurrLightAll ();

	return;
        }  // end of ltypeCB

// extern void setLightSourcePositionDefaults (LightSourceDesc *ls);
void positionResetCB (Fl_Widget *w)
	{
	setLightSourcePositionDefaults (LightSource + CurrentLightNum);
	sgeSendCurrLightAll ();
	reconcilePosGuiToCurrLS();
	}  // end of postionResetCB 



int DirectionInfType = 1;
void dirInfTypeCB (Fl_Widget *w, void *v)
	{
	DirectionInfType = (long int)v;

	reconcileGuiToCurrInfDir ();

	switch (DirectionInfType)
		{
		case 0:
			LightSourceWidgets.inf.hpDirGrp->hide ();
			LightSourceWidgets.inf.xyzDirGrp->show ();
			break;

		case 1:
			LightSourceWidgets.inf.xyzDirGrp->hide();
			LightSourceWidgets.inf.hpDirGrp->show ();
			break;
		}

	// printf ("dirInfTypeCB  %d\n", DirectionInfType);
	}  // end of dirInfTypeCB


void spotDirTypeCB (Fl_Widget *w, void *v)
	{
	LightSource[CurrentLightNum].spotDirectionType = (long int)v;


	reconcileGuiToCurrSpotDir ();

#if 0

The following block of code needs to be in reconcileGuiToCurrSpotDir,
so it doesn,t need to be here.

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

#endif




	}  // end of spotDirTypeCB




// POS
void posButCB (Fl_Widget *w, long id)
	{

	// Fl_Window *win;
	// XYZ_Edit_Window *win;
	Rel_Pos_Window *win;
	int relPos[2];

// printf ("posButCB %d\n", id);






	switch (id)
		{
		case 0:
			// show editing subwindow for spot light position
			win = LightSourceWidgets.spot.xyzPosWin;
			break;

		case 1:
		    // Show the editing subwindow for light direction

		    // OK, first we are going to make all of the widgets that
		    // determine direction agree with what is currently
		    // in the data structure.
		    reconcileGuiToCurrSpotDir ();
		
		    // then switch which window is visible 
		    switch (LightSource[CurrentLightNum].spotDirectionType)
			{
			case 0:
				win = LightSourceWidgets.spot.xyzDirWin;
				break;
			case 1:
				win = LightSourceWidgets.spot.hpDirWin;
				break;
			case 2:
				win = LightSourceWidgets.spot.lookAtDirWin;
				break;
			default:
				return;
			}
		    break;


		case 10:
			// show editing subwindow for omni light position
			win = LightSourceWidgets.local.xyzPosWin;
			break;

		case 11:
		    reconcileGuiToCurrInfDir ();
		    // then switch which window is visible 
		    switch (DirectionInfType)
			{
			case 0:
				win = LightSourceWidgets.inf.xyzDirWin;
				break;
			case 1:
				win = LightSourceWidgets.inf.hpDirWin;
				break;
			default:
				return;
			}
		    break;
		}

	win->getRelPos (relPos);
	win->position (MainWindow->x()+relPos[0], MainWindow->y()+relPos[1]);
	// win->set_non_modal();
	win->set_modal();
	win->show();
	}  // end of posButCB




// 
// callback for Spot Position
//
void 
spotXyzPosAdjCB (Fl_Widget *w, long id)
	{
	
	Fl_Adjuster *adj = LightSourceWidgets.spot.xyzPosWin->adjuster(id);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (w, id);

	// Make the associated Fl_Value_Input agree
	double val = LightSourceWidgets.spot.xyzPosAdj[id]->value() ;
	LightSourceWidgets.spot.xyzPosFI[id]->value (val);

	LightSource[CurrentLightNum].localLocation[id] = val;
	sgeSendCurrLocalLocation ();

	// If we are in lookAt mode, then change the spot direction
	// so that we continue to look at the lookAt point.
	if (LightSource[CurrentLightNum].spotDirectionType == SPOT_DIR_LOOKAT)
		{
		LightSourceDesc *ls = LightSource + CurrentLightNum;
		ls->spotDirection[0] = ls->spotLookAtPt[0]-ls->localLocation[0];
		ls->spotDirection[1] = ls->spotLookAtPt[1]-ls->localLocation[1];
		ls->spotDirection[2] = ls->spotLookAtPt[2]-ls->localLocation[2];
		sgeSendCurrSpotDirection ();
		}

	}  // end of spotXyzPosAdjCB


void 
spotXyzPosFICB (Fl_Widget *w, long id)
	{
	
	float val;
	
	val = LightSourceWidgets.spot.xyzPosFI[id]->value();

	LightSourceWidgets.spot.xyzPosAdj[id]->value(val);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (LightSourceWidgets.spot.xyzPosAdj[id], id);

	LightSource[CurrentLightNum].localLocation[id] = val;
	sgeSendCurrLocalLocation ();

	// If we are in lookAt mode, then change the spot direction
	// so that we continue to look at the lookAt point.
	if (LightSource[CurrentLightNum].spotDirectionType == SPOT_DIR_LOOKAT)
		{
		LightSourceDesc *ls = LightSource + CurrentLightNum;
		ls->spotDirection[0] = ls->spotLookAtPt[0]-ls->localLocation[0];
		ls->spotDirection[1] = ls->spotLookAtPt[1]-ls->localLocation[1];
		ls->spotDirection[2] = ls->spotLookAtPt[2]-ls->localLocation[2];
		sgeSendCurrSpotDirection ();
		}

	}  // end of spotXyzPosFICB



// 
// callback for omni Position
//
void 
localXyzPosAdjCB (Fl_Widget *w, long id)
	{
	
	Fl_Adjuster *adj = LightSourceWidgets.local.xyzPosWin->adjuster(id);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (w, id);

	// Make the associated Fl_Value_Input agree
	double val = LightSourceWidgets.local.xyzPosAdj[id]->value() ;
	LightSourceWidgets.local.xyzPosFI[id]->value (val);

	LightSource[CurrentLightNum].localLocation[id] = val;


	sgeSendCurrLocalLocation ();

	}  // end of localXyzPosAdjCB


void 
localXyzPosFICB (Fl_Widget *w, long id)
	{
	
	float val;
	
	val = LightSourceWidgets.local.xyzPosFI[id]->value();

	LightSourceWidgets.local.xyzPosAdj[id]->value(val);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (LightSourceWidgets.local.xyzPosAdj[id], id);

	LightSource[CurrentLightNum].localLocation[id] = val;

	sgeSendCurrLocalLocation ();

	}  // end of localXyzPosFICB








// 
// callback for lookAt direction using subwindow with Fl_Adjuster widgets
//
void 
lookAtDirAdjCB (Fl_Widget *w, long id)
	{
	
	Fl_Adjuster *adj = LightSourceWidgets.spot.lookAtDirWin->adjuster(id);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (w, id);

	// Make the associated Fl_Value_Input agree
	double val = LightSourceWidgets.spot.lookAtDirAdj[id]->value() ;
	LightSourceWidgets.spot.lookAtDirFI[id]->value (val);;

	


	// Make the light source direction go from the position
	// to the lookAt point.
	LightSource[CurrentLightNum].spotLookAtPt[id] = val;

	LightSource[CurrentLightNum].spotDirection [id] = 
		LightSource[CurrentLightNum].spotLookAtPt[id] - 
		LightSource[CurrentLightNum].localLocation[id] ;

#if 0
	fprintf (stderr, "LookAt:  \n");
	fprintf (stderr, "        loc:    %f %f %f\n", 
		LightSource[CurrentLightNum].localLocation[0],
		LightSource[CurrentLightNum].localLocation[1],
		LightSource[CurrentLightNum].localLocation[2]);
	fprintf (stderr, "        lookAt widgets: %s %s %s\n", 
			LightSourceWidgets.spot.lookAtDirFI[0]->value (),
			LightSourceWidgets.spot.lookAtDirFI[1]->value (),
			LightSourceWidgets.spot.lookAtDirFI[2]->value ());
	fprintf (stderr, "        lookAtPt:    %f %f %f\n", 
		LightSource[CurrentLightNum].spotLookAtPt[0],
		LightSource[CurrentLightNum].spotLookAtPt[1],
		LightSource[CurrentLightNum].spotLookAtPt[2]);
	fprintf (stderr, "        dir:    %f %f %f\n", 
		LightSource[CurrentLightNum].spotDirection[0],
		LightSource[CurrentLightNum].spotDirection[1],
		LightSource[CurrentLightNum].spotDirection[2]);
#endif




	sgeSendCurrSpotDirection ();

	}  // end of lookAtDirAdjCB


void 
lookAtDirFICB (Fl_Widget *w, long id)
	{
	
	float val;
	
	val = LightSourceWidgets.spot.lookAtDirFI[id]->value();

	LightSourceWidgets.spot.lookAtDirAdj[id]->value(val);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (LightSourceWidgets.spot.xyzPosAdj[id], id);

	// Make the light source direction go from the position
	// to the lookAt point.
	LightSource[CurrentLightNum].spotLookAtPt[id] = val;

	LightSource[CurrentLightNum].spotDirection [id] = 
		LightSource[CurrentLightNum].spotLookAtPt[id] - 
		LightSource[CurrentLightNum].localLocation[id] ;



#if 0
	fprintf (stderr, "LookAt:  \n");
	fprintf (stderr, "        loc:    %f %f %f\n", 
		LightSource[CurrentLightNum].localLocation[0],
		LightSource[CurrentLightNum].localLocation[1],
		LightSource[CurrentLightNum].localLocation[2]);
	fprintf (stderr, "        lookAt: %s %s %s\n", 
			LightSourceWidgets.spot.lookAtDirFI[0]->value (),
			LightSourceWidgets.spot.lookAtDirFI[1]->value (),
			LightSourceWidgets.spot.lookAtDirFI[2]->value ());
	fprintf (stderr, "        lookAtPt:    %f %f %f\n", 
		LightSource[CurrentLightNum].spotLookAtPt[0],
		LightSource[CurrentLightNum].spotLookAtPt[1],
		LightSource[CurrentLightNum].spotLookAtPt[2]);
	fprintf (stderr, "        dir:    %f %f %f\n", 
		LightSource[CurrentLightNum].spotDirection[0],
		LightSource[CurrentLightNum].spotDirection[1],
		LightSource[CurrentLightNum].spotDirection[2]);
#endif


	sgeSendCurrSpotDirection ();

	}  // end of lookAtDirFICB








// 
// callbacks for Spot Direction
//
//
// adjuster callback for xyz light direction 
void 
spotXyzDirAdjCB (Fl_Widget *w, long id)
	{
	
	// printf ("spotXyzDirAdjCB\n");

	Fl_Adjuster *adj = LightSourceWidgets.spot.xyzDirWin->adjuster(id);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (w, id);

	// Make the associated Fl_Value_Input agree
	double val = LightSourceWidgets.spot.xyzDirAdj[id]->value() ;
	LightSourceWidgets.spot.xyzDirFI[id]->value (val);;

	LightSource[CurrentLightNum].spotDirection [id] = val;

	// JGH should we make spotLookAtPt consistent with direction?

	sgeSendCurrSpotDirection ();

	}  // end of spotXyzDirAdjCB


//
// Fl_Value_Input callback for xyz light direction 
void 
spotXyzDirFICB (Fl_Widget *w, long id)
	{
	
	float val;
	
	val = LightSourceWidgets.spot.xyzDirFI[id]->value();

	LightSourceWidgets.spot.xyzDirAdj[id]->value(val);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (LightSourceWidgets.spot.xyzDirAdj[id], id);

	LightSource[CurrentLightNum].spotDirection [id] = val;

	// JGH should we make spotLookAtPt consistent with direction?

	sgeSendCurrSpotDirection ();

	}  // end of spotXyzDirFICB




// 
// callbacks for inf light Direction
//
//
// adjuster callback for xyz light direction 
void 
infXyzDirAdjCB (Fl_Widget *w, long id)
	{
	
	// printf ("infXyzDirAdjCB\n");

	Fl_Adjuster *adj = LightSourceWidgets.inf.xyzDirWin->adjuster(id);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (w, id);

	// Make the associated Fl_Value_Input agree
	char s[50];
	double val = LightSourceWidgets.inf.xyzDirAdj[id]->value() ;
	LightSourceWidgets.inf.xyzDirFI[id]->value (val);;

	LightSource[CurrentLightNum].infDirection [id] = val;

	sgeSendCurrInfDirection ();

	}  // end of infXyzDirAdjCB


//
// Fl_Value_Input callback for xyz inf light direction 
void 
infXyzDirFICB (Fl_Widget *w, long id)
	{
	
	float val;
	
	val = LightSourceWidgets.inf.xyzDirFI[id]->value();

	LightSourceWidgets.inf.xyzDirAdj[id]->value(val);

	// Execute the default callback, which makes the label 
	// match the value of the adjuster.
	XYZ_Edit_Window::adjCB (LightSourceWidgets.inf.xyzDirAdj[id], id);

	LightSource[CurrentLightNum].infDirection [id] = val;

	sgeSendCurrInfDirection ();

	}  // end of infXyzDirFICB











#if 0
void hpToXYZ (double hp[2], double xyz[3])
	{
	double radHP[2];

	radHP[0] = (M_PI/180.0) * hp[0];
	radHP[1] = (M_PI/180.0) * hp[1];


	xyz[2] = sin (radHP[1]);
	double s = cos (radHP[1]);

	xyz[0] = sin (radHP[0]) / s;
	xyz[1] = cos (radHP[0]) / s;
	}  // end of hpToXYZ


void xyzToHP (double xyz[3], double hp[2])
	{
	double len = sqrt ( xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2] );

	double unit[3];
	unit[0] = xyz[0]/len;
	unit[1] = xyz[1]/len;
	unit[2] = xyz[2]/len;

	hp[0] = (180.0/M_PI) * atan2 (unit[1], unit[0]);

	hp[1] = asin (unit[2]);
	}  // end of xyzToHP
#endif



//
// float input callback for hp light direction
void 
spotHpDirFICB (Fl_Widget *w, long id)
	{

	double hp[2];

	hp[0] = LightSourceWidgets.spot.hpDirFI[0]->value();
	hp[1] = LightSourceWidgets.spot.hpDirFI[1]->value();

	if (id == 0)
		{
		LightSourceWidgets.spot.hpDirPos->xvalue(hp[id]);
		}
	else
		{
		LightSourceWidgets.spot.hpDirPos->yvalue(hp[id]);
		}

	HP_Edit_Window::hpCB (LightSourceWidgets.spot.hpDirPos, 0);


	// convert from hp to xyz and put into current light
	double xyz[3];
	hp[0] = 90 - hp[0];
	hpToXYZ (hp, xyz);
	LightSource[CurrentLightNum].spotDirection[0] = xyz[0];
	LightSource[CurrentLightNum].spotDirection[1] = xyz[1];
	LightSource[CurrentLightNum].spotDirection[2] = xyz[2];

	// JGH should we make spotLookAtPt consistent with direction?

	sgeSendCurrSpotDirection ();

	}  // end of spotHpDirFICB

//
// positioner callback for hp light direction
void
spotHpDirPosCB (Fl_Widget *w, long id)
	{

	// Execute the default callback, which makes the label 
	// match the value of the positioner
	HP_Edit_Window::hpCB (LightSourceWidgets.spot.hpDirPos, 0);

	// Make the associated Fl_Value_Input agree
	double hp[2];
	hp[0] = LightSourceWidgets.spot.hpDirPos->xvalue ();
	hp[1] = LightSourceWidgets.spot.hpDirPos->yvalue ();

	LightSourceWidgets.spot.hpDirFI[0]->value (hp[0]);
	LightSourceWidgets.spot.hpDirFI[1]->value (hp[1]);




	// convert from hp to xyz and put into current light
	double xyz[3];
	hp[0] = 90 - hp[0];
	hpToXYZ (hp, xyz);
	LightSource[CurrentLightNum].spotDirection[0] = xyz[0];
	LightSource[CurrentLightNum].spotDirection[1] = xyz[1];
	LightSource[CurrentLightNum].spotDirection[2] = xyz[2];

	// JGH should we make spotLookAtPt consistent with direction?

	sgeSendCurrSpotDirection ();

	}  // end of spotHpDirPosCB



//
// float input callback for hp inf light direction
void 
infHpDirFICB (Fl_Widget *w, long id)
	{

	double hp[2];

	hp[0] = LightSourceWidgets.inf.hpDirFI[0]->value();
	hp[1] = LightSourceWidgets.inf.hpDirFI[1]->value();

	if (id == 0)
		{
		LightSourceWidgets.inf.hpDirPos->xvalue(hp[id]);
		}
	else
		{
		LightSourceWidgets.inf.hpDirPos->yvalue(hp[id]);
		}

	HP_Edit_Window::hpCB (LightSourceWidgets.inf.hpDirPos, 0);


	// convert from hp to xyz and put into current light
	double xyz[3];
	hp[0] -= 90;
	hpToXYZ (hp, xyz);
	LightSource[CurrentLightNum].infDirection[0] = xyz[0];
	LightSource[CurrentLightNum].infDirection[1] = xyz[1];
	LightSource[CurrentLightNum].infDirection[2] = xyz[2];


	sgeSendCurrInfDirection ();

	}  // end of infHpDirFICB

//
// positioner callback for hp light direction
void
infHpDirPosCB (Fl_Widget *w, long id)
	{

	// Execute the default callback, which makes the label 
	// match the value of the positioner
	HP_Edit_Window::hpCB (LightSourceWidgets.inf.hpDirPos, 0);

	// Make the associated Fl_Value_Input agree
	double hp[2];
	hp[0] = LightSourceWidgets.inf.hpDirPos->xvalue ();
	hp[1] = LightSourceWidgets.inf.hpDirPos->yvalue ();

	LightSourceWidgets.inf.hpDirFI[0]->value (hp[0]);;
	LightSourceWidgets.inf.hpDirFI[1]->value (hp[1]);;




	// convert from hp to xyz and put into current light
	double xyz[3];
	hp[0] -= 90;
	hpToXYZ (hp, xyz);
	LightSource[CurrentLightNum].infDirection[0] = xyz[0];
	LightSource[CurrentLightNum].infDirection[1] = xyz[1];
	LightSource[CurrentLightNum].infDirection[2] = xyz[2];


	sgeSendCurrInfDirection ();

	}  // end of infHpDirPosCB







static void
angleExpFICB (Fl_Widget *w, long id)
	{
	Fl_Value_Input *fi = (id == 0) ?
		LightSourceWidgets.spot.angleFI  :
		LightSourceWidgets.spot.expFI  ;

	Fl_Scrollbar *sb = (id == 0) ?
		LightSourceWidgets.spot.angleSB  :
		LightSourceWidgets.spot.expSB  ;

	
 	double val;
	val = fi->value ();

	((Fl_Slider *)sb)->value (val);


	if (id == 0)
		{
		LightSource[CurrentLightNum].spotCutoff= val;
		}
	else
		{
		LightSource[CurrentLightNum].spotExponent= val;
		}
		
	sgeSendCurrSpotExpAng ();


	}  // end of angleExpFICB


static void
angleExpSBCB (Fl_Widget *w, long id)
	{
	Fl_Value_Input *fi = (id == 0) ?
		LightSourceWidgets.spot.angleFI  :
		LightSourceWidgets.spot.expFI  ;

	Fl_Scrollbar *sb = (id == 0) ?
		LightSourceWidgets.spot.angleSB  :
		LightSourceWidgets.spot.expSB  ;

	char s[50];
	double val = sb->value();
	int precision;
	// sprintf (s, "%.4f", val);

	if (id == 0)
		{
		LightSource[CurrentLightNum].spotCutoff = val;
		precision = 2;
		}
	else
		{
		LightSource[CurrentLightNum].spotExponent= val;
		precision = 3;
		}
		
#if 0
	formatDouble (val, precision, s);
	fi->value (s);
#else
	fi->value (val);
#endif

	sgeSendCurrSpotExpAng ();
	}  // end of angleExpFICB

static void
attenCB (Fl_Widget *w, long id)
	{

#if 0
	LightSource[CurrentLightNum].atten[id] = ((Fl_Value_Input *)w)->value();
#else
	double val = ((Fl_Value_Input *)w)->value();
	if (val >= 0)
		{
		LightSource[CurrentLightNum].atten[id] = val;
		}
	else
		{
		((Fl_Value_Input *)w)->value(0.0);
		LightSource[CurrentLightNum].atten[id] = 0.0;
		}
		

#endif

	sgeSendCurrAttenuations ();

	}  // end of attenCB



//
// End of callbacks
//
///////////////////////////////////////////////////////////////////////////
//
// Build the GUI
//


static Fl_Group *buildFltInputGrp (int n, int x, int y, int w, int h, 
			double step, double min, double max, int soft)
	{
	Fl_Group *fltGrp = new Fl_Group (x, y, w, h);
	fltGrp->end();

	int spacing = 5;
	int widgetWidth = (w - (n-1)*spacing) / n;
	Fl_Value_Input *inp;

	for (int i = 0; i < n; i++)
		{
		int currX = x + i * (widgetWidth + spacing);
		inp = new Fl_Value_Input (currX, y, widgetWidth, h);
		inp->step (step);
		inp->range (min, max);
		inp->soft (soft);
		fltGrp->add (inp);
		}


	return fltGrp;
	}   // end of buildFltInputGrp

static Fl_Group *buildValInputGrp (int n, int x, int y, int w, int h)
	{
	Fl_Group *fltGrp = new Fl_Group (x, y, w, h);
	fltGrp->end();

	int spacing = 5;
	int widgetWidth = (w - (n-1)*spacing) / n;
	Fl_Value_Input *inp;

	for (int i = 0; i < n; i++)
		{
		int currX = x + i * (widgetWidth + spacing);
		inp = new Fl_Value_Input (currX, y, widgetWidth, h);
		// we're handling step, range, and soft elsewhere
		fltGrp->add (inp);
		}


	return fltGrp;
	}   // end of buildValInputGrp





// POS
static Fl_Group *buildSingleHorizFltEntry (
			int x, int y, int w, int h, 
			const char *labelText, 
			double min, double max, 
			double step, int soft)
	{

	Fl_Group *grp = new Fl_Group (x, y, w, h);
	grp->end ();


	Fl_Box *lab = new Fl_Box (x, y, 80, h, labelText);
	lab->align (FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	grp->add (lab);

	Fl_Value_Input *input = new Fl_Value_Input (x+85, y, 85, 30);
	input->step (step);
	input->range (min, max);
	input->soft (soft);
	grp->add (input);

	Fl_Scrollbar *sb = new Fl_Scrollbar (x+180, y+5, x+w-200, 20);
	sb->range (min, max);
	sb->step (step);
	sb->type (FL_HORIZONTAL);
	grp->add (sb);

	return grp;
	}  // end of buildSingleHorizFltEntry






//
// Get the n'th Fl_Value_Input widget in the group.
// Not efficient, but what the heck.
Fl_Value_Input *getFloatInputChild (Fl_Group *grp, int index)
	{

	if (index < 0)
		{
		return NULL;
		}

	Fl_Widget * const *children = grp->array ();
	if (children == NULL)
		{
		return NULL;
		}

	int n = grp->children ();
	int iFI = 0;
	for (int i = 0; i < n; i++)
		{
		Fl_Value_Input *fi = 
		      dynamic_cast <Fl_Value_Input *>(children[i]);
		if (fi != NULL)
			{
			if (iFI == index)
				{
				return fi;
				}
			iFI++;
			}
		}

	return NULL;
	} // end of getFloatInputChild





//
// Get the n'th Fl_Scrollbarwidget in the group.
// Not efficient, but what the heck.
Fl_Scrollbar *getScrollbarChild (Fl_Group *grp, int index)
	{

	if (index < 0)
		{
		return NULL;
		}

	Fl_Widget * const *children = grp->array ();
	if (children == NULL)
		{
		return NULL;
		}

	int n = grp->children ();
	int iFI = 0;
	for (int i = 0; i < n; i++)
		{
		Fl_Scrollbar *fi = 
		      dynamic_cast <Fl_Scrollbar *>(children[i]);
		if (fi != NULL)
			{
			if (iFI == index)
				{
				return fi;
				}
			iFI++;
			}
		}

	return NULL;
	} // end of getScrollbarChild








static Fl_Group *buildInfiniteLightGroup (int x, int y, int w, int h)
	{
	Fl_Group *infGroup  = new Fl_Group (x, y, w, h);
	infGroup->end ();

	int currX = x+3;
	int currY = y+3;

	Fl_Button *dBut = new Fl_Button (currX, currY, 80, 30, "Direction");
	dBut->callback (posButCB, 11);
	infGroup->add (dBut); 


  	static Fl_Menu_Item dirList[] =
		{
			{" X Y Z ", 0,     dirInfTypeCB, (void *)0},
			{"Angles", 0,       dirInfTypeCB, (void *)1},
			{NULL}
		};

	LightSourceWidgets.inf.dirMenu = 
			new Fl_Choice (currX+85, currY, 85, 30);
	LightSourceWidgets.inf.dirMenu->menu (dirList);
	infGroup->add (LightSourceWidgets.inf.dirMenu);
	LightSourceWidgets.inf.dirMenu->value (DirectionInfType);

	LightSourceWidgets.inf.xyzDirGrp = 
		buildFltInputGrp (3, currX+180, currY, w+x-(currX+185), 					30, 0.001, -10.0, 10.0, 1);
	infGroup->add (LightSourceWidgets.inf.xyzDirGrp);
	LightSourceWidgets.inf.xyzDirGrp->hide ();

	LightSourceWidgets.inf.hpDirGrp = 
		buildFltInputGrp (2, currX+180, currY, w+x-(currX+185),
 					30, 0.01, -180.0, 180.0, 0);
	infGroup->add (LightSourceWidgets.inf.hpDirGrp);
	LightSourceWidgets.inf.hpDirGrp->show ();

	LightSourceWidgets.inf.xyzDirWin = 
	 new Dir_Edit_Window (currX+180, currY+50, 200, 200, 
		"Light Direction Vector");

	LightSourceWidgets.inf.hpDirWin = 
	 new HP_Edit_Window (currX+180, currY+50, 200, 200, 
		"Light H-P Direction");

	return infGroup;
	}  // end of buildInfiniteLightGroup



static Fl_Group *buildLocalLightGroup (int x, int y, int w, int h)
	{
	Fl_Group *localGroup  = new Fl_Group (x, y, w, h);
	localGroup->end ();

	int currX = x+3;
	int currY = y+3;

	Fl_Button *pBut = new Fl_Button (currX, currY, 80, 30, "Location");
	pBut->callback (posButCB, 10);
	localGroup->add (pBut); 

	Fl_Box *pLab2 = new Fl_Box (currX+85, currY, 85, 30, "X Y Z");
	localGroup->add (pLab2); 


	LightSourceWidgets.local.xyzPosGrp = 
	    buildFltInputGrp (3, currX+180, currY, w+x-(currX+185), 
					30, 0.001, -10.0, 10.0, 1);
	localGroup->add (LightSourceWidgets.local.xyzPosGrp);

	LightSourceWidgets.local.xyzPosWin = 
	  new XYZ_Edit_Window (currX+180, currY+50, 200, 200, "Light Location");


	currY += 35;




	return localGroup;
	}  // end of buildLocalLightGroup


void
resolveLocalLightCallbacks (LocalWidgets *local)
	{
	for (int i = 0; i < 3; i++)
		{
		local->xyzPosAdj[i] = local->xyzPosWin->adjuster (i);
		local->xyzPosFI[i] = getFloatInputChild (local->xyzPosGrp, i);

		local->xyzPosAdj[i]->callback (localXyzPosAdjCB, i);
		local->xyzPosFI[i]->callback (localXyzPosFICB, i);
		}

	}  // end of resolveLocalLightCallbacks


void
resolveInfLightCallbacks (InfWidgets *inf)
	{

	for (int i = 0; i < 3; i++)
		{
		inf->xyzDirAdj[i] = inf->xyzDirWin->adjuster (i);
		inf->xyzDirFI[i] = getFloatInputChild (inf->xyzDirGrp, i);

		inf->xyzDirAdj[i]->callback (infXyzDirAdjCB, i);
		inf->xyzDirFI[i]->callback (infXyzDirFICB, i);
		}

	// light HP direction  widgets
	inf->hpDirFI[0] = getFloatInputChild (inf->hpDirGrp, 0);
	inf->hpDirFI[1] = getFloatInputChild (inf->hpDirGrp, 1);
	inf->hpDirFI[1]->range (-90.0, 90.0);
	inf->hpDirPos = inf->hpDirWin->getPositioner ();

	inf->hpDirFI[0]->callback (infHpDirFICB, 0);
	inf->hpDirFI[1]->callback (infHpDirFICB, 1);
	inf->hpDirPos->callback (infHpDirPosCB, 0);


	}  // end of resolveInfLightCallbacks

static void
resolveSpotCallbacks (SpotWidgets *spot)
	{
	// We assume that the subwindows have been created but that
	// only the default callbacks within the subwindows are
	// in place.

	// This loop resolves widgets for 
	//	light position 
	//	light xyz direction 
	//	light look at direction
	for (int i = 0; i < 3; i++)
		{
		spot->xyzPosAdj[i] = spot->xyzPosWin->adjuster (i);
		spot->xyzPosFI[i] = getFloatInputChild (spot->xyzPosGrp, i);

		spot->xyzPosAdj[i]->callback (spotXyzPosAdjCB, i);
		spot->xyzPosFI[i]->callback (spotXyzPosFICB, i);


		spot->xyzDirAdj[i] = spot->xyzDirWin->adjuster (i);
		spot->xyzDirFI[i] = getFloatInputChild (spot->xyzDirGrp, i);

		spot->xyzDirAdj[i]->callback (spotXyzDirAdjCB, i);
		spot->xyzDirFI[i]->callback (spotXyzDirFICB, i);


		spot->lookAtDirAdj[i] = spot->lookAtDirWin->adjuster (i);
		spot->lookAtDirFI[i]=getFloatInputChild (spot->lookAtDirGrp, i);

		spot->lookAtDirAdj[i]->callback (lookAtDirAdjCB, i);
		spot->lookAtDirFI[i]->callback (lookAtDirFICB, i);

		}

	// light HP direction  widgets
	spot->hpDirFI[0] = getFloatInputChild (spot->hpDirGrp, 0);
	spot->hpDirFI[1] = getFloatInputChild (spot->hpDirGrp, 1);
	spot->hpDirFI[1]->range (-90.0, 90.0);
	spot->hpDirPos = spot->hpDirWin->getPositioner ();

	spot->hpDirFI[0]->callback (spotHpDirFICB, 0);
	spot->hpDirFI[1]->callback (spotHpDirFICB, 1);
	spot->hpDirPos->callback (spotHpDirPosCB, 0);



	spot->angleFI = getFloatInputChild (spot->angleGrp, 0);
	spot->angleSB = getScrollbarChild (spot->angleGrp, 0);
	spot->angleFI->callback (angleExpFICB, 0);
	spot->angleSB->callback (angleExpSBCB, 0);

	spot->expFI = getFloatInputChild (spot->expGrp, 0);
	spot->expSB = getScrollbarChild (spot->expGrp, 0);
	spot->expFI->callback (angleExpFICB, 1);
	spot->expSB->callback (angleExpSBCB, 1);



	}  // end of resolveSpotCallbacks



// POS
static Fl_Group *buildSpotLightGroup (int x, int y, int w, int h)
	{
	Fl_Group *spotGroup  = new Fl_Group (x, y, w, h);
	spotGroup->end ();

	int currX = x+3;
	int currY = y+3;




	// Position
#if 0
	Fl_Box *pLab = new Fl_Box (currX, currY, 80, 30, "Position");
	pLab->align (FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	spotGroup->add (pLab); 
#else
	Fl_Button *pBut = new Fl_Button (currX, currY, 80, 30, "Location");
	pBut->callback (posButCB, 0);
	spotGroup->add (pBut); 
#endif
	Fl_Box *pLab2 = new Fl_Box (currX+85, currY, 85, 30, "X Y Z");
	spotGroup->add (pLab2); 


	LightSourceWidgets.spot.xyzPosGrp = 
	    buildFltInputGrp (3, currX+180, currY, w+x-(currX+185), 
					30, 0.001, -10.0, 10.0, 1);
	spotGroup->add (LightSourceWidgets.spot.xyzPosGrp);

	LightSourceWidgets.spot.xyzPosWin = 
	  new XYZ_Edit_Window (currX+180, currY+50, 200, 200, "Light Position");


	currY += 35;



	// Direction
#if 0
	Fl_Box *dLab = new Fl_Box (currX, currY, 80, 30, "Direction");
	dLab->align (FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	spotGroup->add (dLab); 
#else
	Fl_Button *dBut = new Fl_Button (currX, currY, 80, 30, "Direction");
	dBut->callback (posButCB, 1);
	spotGroup->add (dBut); 

#endif

  	static Fl_Menu_Item dirList[] =
		{
			{" X Y Z ", 0,     spotDirTypeCB, (void *)0},
			{"Angles", 0,       spotDirTypeCB, (void *)1},
			{"LookAt", 0,    spotDirTypeCB, (void *)2},
			{NULL}
		};

	LightSourceWidgets.spot.dirMenu = 
			new Fl_Choice (currX+85, currY, 85, 30);
	LightSourceWidgets.spot.dirMenu->menu (dirList);
	LightSourceWidgets.spot.dirMenu->
		value (LightSource[CurrentLightNum].spotDirectionType);
	spotGroup->add (LightSourceWidgets.spot.dirMenu);

	LightSourceWidgets.spot.xyzDirGrp = 
		buildFltInputGrp (3, currX+180, currY, w+x-(currX+185), 
					30, 0.001, -10.0, 10.0, 1);
	spotGroup->add (LightSourceWidgets.spot.xyzDirGrp);
	LightSourceWidgets.spot.xyzDirGrp->hide ();

	LightSourceWidgets.spot.hpDirGrp = 
		buildFltInputGrp (2, currX+180, currY, w+x-(currX+185), 
					30, 0.01, -180.0, 180.0, 0);
	spotGroup->add (LightSourceWidgets.spot.hpDirGrp);
	LightSourceWidgets.spot.hpDirGrp->show ();

	LightSourceWidgets.spot.lookAtDirGrp = 
		buildFltInputGrp (3, currX+180, currY, w+x-(currX+185), 
					30, 0.001, -10.0, 10.0, 1);
	spotGroup->add (LightSourceWidgets.spot.lookAtDirGrp);
	LightSourceWidgets.spot.lookAtDirGrp->hide ();


	LightSourceWidgets.spot.xyzDirWin = 
	 new Dir_Edit_Window (currX+180, currY+50, 200, 200, 
		"Light Direction Vector");

	LightSourceWidgets.spot.hpDirWin = 
	 new HP_Edit_Window (currX+180, currY+50, 200, 200, 
		"Light H-P Direction");

	LightSourceWidgets.spot.lookAtDirWin = 
	  new XYZ_Edit_Window (currX+180, currY+50, 200, 200, 
		"Light LookAt Direction");

	// printf ("build lookAtDirWin = %p\n", LightSourceWidgets.spot.lookAtDirWin);






	currY += 35;






	LightSourceWidgets.spot.angleGrp = buildSingleHorizFltEntry (
		currX, currY, w+x-currX, 30, 
		"SpreadAng", 0.0, 90.0, 0.1, 0);

	spotGroup->add (LightSourceWidgets.spot.angleGrp);



	currY += 35;


	LightSourceWidgets.spot.expGrp = buildSingleHorizFltEntry (
		currX, currY, w+x-currX, 30, 
		"Exponent", 0.0, 128.0, 0.1, 0);

	spotGroup->add (LightSourceWidgets.spot.expGrp);











#if 1
	resolveSpotCallbacks (&(LightSourceWidgets.spot));
	resolveInfLightCallbacks (&(LightSourceWidgets.inf));
	resolveLocalLightCallbacks (&(LightSourceWidgets.local));
#endif
	

	return spotGroup;
	}  // end of buildSpotLightGroup




// POS
static Fl_Group *buildAttenGroup (int x, int y, int w, int h)
	{
	
	Fl_Group *attenGrp = new Fl_Group (x, y, w, h);
	attenGrp->end ();

	int currX = x;
	int currY = y;

	Fl_Box *aLab = new Fl_Box (currX, currY+30, 85, 30, "Attenuation");
	aLab->align (FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
	attenGrp->add (aLab); 

	Fl_Group *attenInputGrp =  LightSourceWidgets.attenInputGrp = 
	    buildValInputGrp (3, currX+85, currY+30, w+x-(currX+90), 30);
	attenGrp->add (LightSourceWidgets.attenInputGrp);


	LightSourceWidgets.attenVI[0] = 
		(Fl_Value_Input *) attenInputGrp->child(0);
	LightSourceWidgets.attenVI[0]->range (0.0, 1.0); 
	LightSourceWidgets.attenVI[0]->step (0.001); 
	LightSourceWidgets.attenVI[0]->soft (1); 

	LightSourceWidgets.attenVI[1] = 
		(Fl_Value_Input *) attenInputGrp->child(1);
	LightSourceWidgets.attenVI[1]->range (0.0, 10.0); 
	LightSourceWidgets.attenVI[1]->step (0.001); 
	LightSourceWidgets.attenVI[1]->soft (1); 

	LightSourceWidgets.attenVI[2] = 
		(Fl_Value_Input *) attenInputGrp->child(2);
	LightSourceWidgets.attenVI[2]->range (0.0, 10.0); 
	LightSourceWidgets.attenVI[2]->step (0.001); 
	LightSourceWidgets.attenVI[2]->soft (1); 



	LightSourceWidgets.attenVI[0]->callback (attenCB, 0); 
	LightSourceWidgets.attenVI[1]->callback (attenCB, 1); 
	LightSourceWidgets.attenVI[2]->callback (attenCB, 2); 


	int nChildren = attenInputGrp->children();

	static const char *attenText[3] = {"constant", "linear", "quadratic"};
	for (int i = 0; i < MIN (3, nChildren); i++)
		{
		Fl_Widget *w = attenInputGrp->child (i);


		Fl_Box *lab = 
		  new Fl_Box (w->x(), currY+10, w->w(), w->h(), attenText[i]);
		attenGrp->add (lab);
		}







	return attenGrp;
	}  // end of buildAttenGroup


// POS
Fl_Group *buildPositionEditGroup (int x, int y, int w, int h)
	{
	Fl_Group *posGroup = new Fl_Group (x, y, w, h);
	posGroup->end ();
	// posGroup->box (FL_BORDER_BOX);
        posGroup->box (FL_SHADOW_BOX);



  	static Fl_Menu_Item lightTypeList[] =
		{
			{"Infinite", 0,               ltypeCB, (void *)0},
			{"Omni-directional", 0,       ltypeCB, (void *)1},
			{"Spotlight", 0,              ltypeCB, (void *)2},
			{NULL}
		};

	int currX = x+3;
	int currY = y+8;

	Fl_Box *lLabel = new Fl_Box (currX, currY, 100, 30, "Light type: ");
	lLabel->labelfont (FL_HELVETICA_ITALIC);
	lLabel->align (FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	posGroup->add (lLabel);


	Fl_Choice *ltypeMenu = new Fl_Choice (currX+85, currY, 150, 30);
	ltypeMenu->menu (lightTypeList);
	posGroup->add (ltypeMenu);
	LightSourceWidgets.ltypeMenu = ltypeMenu;
// fprintf (stderr, "ltypeMenu = %p\n", LightSourceWidgets.ltypeMenu);

	Fl_Button *resetButton = 
		new Fl_Button (currX+250, currY, 130, 30, "Reset");
        // resetButton->deactivate();
	resetButton->callback (positionResetCB);
        posGroup->add (resetButton);



	currY += 35;


	LightSourceWidgets.inf.group = 
		buildInfiniteLightGroup (currX, currY, w+x-currX, 160);
	// LightSourceWidgets.inf.group->tooltip ("infGrp");
	

#if 1
	LightSourceWidgets.local.group = 
		buildLocalLightGroup (currX, currY, w+x-currX, 160);
	// LightSourceWidgets.local.group->tooltip ("localGrp");
	

#if 1
	LightSourceWidgets.spot.group =
		buildSpotLightGroup (currX, currY, w+x-currX, 160);
	// LightSourceWidgets.spot.group->tooltip ("spotGrp");
#endif



#endif
	


	posGroup->add (LightSourceWidgets.inf.group);


	// posGroup->tooltip ("posGroup");

#if 1
	posGroup->add (LightSourceWidgets.local.group );
	posGroup->add (LightSourceWidgets.spot.group );



#if 1
	LightSourceWidgets.inf.group->show ();
	LightSourceWidgets.local.group->hide ();
	LightSourceWidgets.spot.group->hide ();
#endif

#endif

	currY += 160;


	Fl_Box *sep1 = 
		new Fl_Box (H_CENTER_LINE_BOX, currX+50, currY, w-100, 3, "");
	posGroup->add (sep1);





	Fl_Group *attenGrp = 
		buildAttenGroup (currX, currY, w+x-currX, h+y-currY);
	posGroup->add (attenGrp);
	LightSourceWidgets.attenGrp = attenGrp;
	



	return posGroup;
	}  // end of buildPositionEditGroup





