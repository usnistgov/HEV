

#ifndef GENERAL_GUI_S_H
#define GENERAL_GUI_S_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Positioner.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_File_Chooser.H>



#define MAX(x,y)   (((x)>(y))?(x):(y))
#define MIN(x,y)   (((x)<(y))?(x):(y))
#define CLAMP01(x)   MIN(MAX((x),0.0),1.0)

#define STD_V_LINE_HEIGHT (30);
#define STD_V_LINE_SPACING (35);
#define MIN_V_LINE_HEIGHT (20);
#define MIN_V_LINE_SPACING (25);


extern void normalize3 (double v[3]);

extern double vecLen3 (double v[3]);

extern double dot (double a[3], double b[3]);

extern void hpToXYZ (double hp[2], double xyz[3]);

extern void xyzToHP (double xyz[3], double hp[2]);


extern int initGUI ();

extern void reconcileGuiToCurrSpotDir ();
extern void reconcileGuiToCurrInfDir ();
extern void reconcileGuiToCurrLocalPos ();
extern void reconcilePosGuiToCurrLS ();

extern int reconcileGuiToCurrentLightSource ();

extern int reconcileGuiToLightModel ();

extern int reconcileGuiToData ();

extern void printEventInfo ();





// ----------------- Separator ------------------------

// Here are some auxiliary functions to help with separators


#define H_CENTER_LINE_BOX  FL_FREE_BOXTYPE
#define V_CENTER_LINE_BOX  ((Fl_Boxtype)(FL_FREE_BOXTYPE+1))

extern void horizontalBoxCenterLineDraw (int x, int y, int w, int h, Fl_Color c);

extern void verticalBoxCenterLineDraw (int x, int y, int w, int h, Fl_Color c);











////////////////////////////////////////////////////////////////////////

class Rel_Pos_Window : public Fl_Window
{
protected:
	int relativePosition[2];

public:
	Rel_Pos_Window (int x, int y, int w, int h, const char *title = 0);

	void getRelPos (int pos[2]);
	void setRelPos (int pos[2]);

};  // end of class Rel_Pos_Window

class Msg_Window : public Rel_Pos_Window
{
protected:
	Fl_Box *msgLabel;
	Fl_Button *doneButton;
	static void cb (Fl_Widget *w, void *v);
public:
	Msg_Window (int x, int y, int w, int h, const char *title = 0);
	void setMsg (char *msg);
};  // end of Msg_Window

class HP_Edit_Window : public Rel_Pos_Window
{

protected:
	 Fl_Positioner *positioner;
	 Fl_Box *hpLabel[2];

public:

	HP_Edit_Window (int x, int y, int w, int h, const char *title = 0);

	Fl_Positioner *getPositioner() {return positioner;};
	// void value (float *x, float *y) {positioner->value(x, y);};
	void value (float *x, float *y) {*x = positioner->xvalue(); *y = positioner->yvalue();};

	static void hpCB (Fl_Widget *w, long id);

};  // end of HP_Edit_Window


// class XYZ_Edit_Window : public Fl_Window
class XYZ_Edit_Window : public Rel_Pos_Window
{

protected:
	Fl_Box *xyzLabel[3];
	Fl_Adjuster *xyzAdj[3];
	char xyzTxtVal[3][30];
	double xyz[3];
	// int relativePosition[2];
	
	double adjRange[2];
	double adjStep;


public:
	static void adjCB (Fl_Widget *w, long id);

	XYZ_Edit_Window (int x, int y, int w, int h, const char *title = 0);

	
	void getXYZVals (double xyz[3]);
	void setXYZVals (double xyz[3]);


	void range (double min, double max);
	void step (double *s);

	Fl_Adjuster *adjuster (int index);
	

};  // end of XYZ_Edit_Window

class Dir_Edit_Window : public XYZ_Edit_Window
{

public:

	Dir_Edit_Window (int x, int y, int w, int h, const char *title = 0);


};   // end of Dir_Edit_Window

typedef struct spot_widgets
	{
	Fl_Group *group;


	Fl_Group *xyzPosGrp;
	Fl_Value_Input *xyzPosFI[3];

	XYZ_Edit_Window *xyzPosWin;
	Fl_Adjuster *xyzPosAdj[3];


	Fl_Choice *dirMenu;

	Fl_Group *xyzDirGrp;
	Fl_Value_Input *xyzDirFI[3];

	Dir_Edit_Window *xyzDirWin;
	Fl_Adjuster *xyzDirAdj[3];


	Fl_Group *hpDirGrp;
	HP_Edit_Window *hpDirWin;
	Fl_Value_Input *hpDirFI[2];
	Fl_Positioner *hpDirPos;


	Fl_Group *lookAtDirGrp;
	Fl_Value_Input *lookAtDirFI[3];

	XYZ_Edit_Window *lookAtDirWin;
	Fl_Adjuster *lookAtDirAdj[3];

	Fl_Group *angleGrp;
	Fl_Value_Input *angleFI;
	Fl_Scrollbar *angleSB;

	Fl_Group *expGrp;
	Fl_Value_Input *expFI;
	Fl_Scrollbar *expSB;


	} SpotWidgets;

typedef struct inf_widgets
	{
	Fl_Group *group;

	Fl_Choice *dirMenu;

	Fl_Group *xyzDirGrp;
	Fl_Value_Input *xyzDirFI[3];

	Dir_Edit_Window *xyzDirWin;
	Fl_Adjuster *xyzDirAdj[3];


	Fl_Group *hpDirGrp;
	HP_Edit_Window *hpDirWin;
	Fl_Value_Input *hpDirFI[2];
	Fl_Positioner *hpDirPos;


	} InfWidgets;

typedef struct local_widgets
	{
	Fl_Group *group;

	Fl_Group *lightPos;

	Fl_Group *xyzPosGrp;
	Fl_Value_Input *xyzPosFI[3];

	XYZ_Edit_Window *xyzPosWin;
	Fl_Adjuster *xyzPosAdj[3];


	} LocalWidgets;






typedef struct lightsourcewidgets
	{

	
	// selects which light source
	Fl_Choice *lnumMenu;


	// General widgets
	Fl_Light_Button *onOff;
	Fl_Choice *nodeMenu;
	Fl_Light_Button *showGlyph;

	Fl_Round_Button *positionMode;
	Fl_Round_Button *colorMode;




	// Color widgets
	Fl_Scrollbar *rgbScrollbar[3];
	Fl_Scrollbar *grayScrollbar;
	Fl_Group *rgbColorGroup[3];
	Fl_Group *grayColorGroup[3];
	Fl_Value_Input *rgbInput[3][3];  // first index is amb/dif/spec
	Fl_Value_Input *grayInput[3];
	Fl_Light_Button *linkAmbDiffSpec;  // can query state from widget
	Fl_Roller *scaleRGB;  
	Fl_Light_Button *rgbEditMode;  // can query state from widget
	Fl_Group *rgbEditGroup;
	Fl_Group *grayEditGroup;



	Fl_Choice *ltypeMenu;

	SpotWidgets spot;
	LocalWidgets local;
	InfWidgets inf;


	Fl_Group *attenGrp;
	Fl_Group *attenInputGrp;
	Fl_Value_Input *attenVI[3];

	}  LightSourceWidgetsDesc;

typedef struct lightmodelwidgets
	{
	// whatever they are
	Fl_Light_Button *colorControl;  // Separate Spec: ON/OFF
	Fl_Light_Button *localViewer;
	Fl_Light_Button *twoSided;

	Fl_Light_Button *rgbEditMode;

	Fl_Roller *scaleRGB;  
	Fl_Group *rgbColorGroup;
	Fl_Group *grayColorGroup;
	Fl_Value_Input *rgbInput[3];
	Fl_Value_Input *grayInput;
	Fl_Scrollbar *rgbScrollbar[3];
	Fl_Scrollbar *grayScrollbar;

	Fl_Group *rgbEditGroup;
	Fl_Group *grayEditGroup;

	}  LightModelWidgetsDesc;


extern Fl_Window *MainWindow;



// GEN
extern LightModelWidgetsDesc LightModelWidgets;

extern LightSourceWidgetsDesc LightSourceWidgets;



// GEN
extern Fl_Group *ColorEditGroup;
extern Fl_Group *PositionEditGroup;

extern int runGUI (int argc, char **argv);




//
// Global Data light source and light model data used
// throughout hev-lightEditor.
//

// DATA
typedef struct lightmodel
	{
	double ambient[4];
	int localViewer;
	int twoSided;


	// Here are some comments on colorControL:
	int colorControlSeparate;
	/*
	GL_LIGHT_MODEL_COLOR_CONTROL (parameter)
              
		parameter value must be either 
		GL_SEPARATE_SPECULAR_COLOR or GL_SINGLE_COLOR. 

		GL_SINGLE_COLOR specifies that a single color is 
		generated from the lighting computation for a vertex. 

		GL_SEPARATE_SPECULAR_COLOR specifies that the specular 
		color computation of lighting be stored separately from 
		the remainder of the lighting computation. 
		The specular color is summed into the generated fragment's 
		color after the application of texture mapping (if enabled). 
		The initial value is GL_SINGLE_COLOR.
    					
		Only in OpenGL 1.2 or later.
	*/

	int inRgbMode;
	} LightModelDesc;

#define INFINITE 0
#define OMNI     1
#define SPOT     2

typedef struct lightsource
	{
	// these correspond to the OpenGL parameters
	char name[500];
	int glLightNum;
	double color[3][4];  // first index: 0 ambient, 1 diffuse, 2 spec

	int type ;   // INFINITE, OMNI, SPOT

#if 0
	double position[4];  // if w = 0, then it's and infinite light source
				// if w != 0, then it's a local light
#endif


	double infDirection[3]; // send as position, but
				// always with w as 0


#if 0
	double constantAtten;
	double linearAtten;
	double quadraticAtten;
#endif

	double atten[3];  // constant, linear, quadratic

	double localLocation[3];  // send as position, 
				  //  but always with w as 1.0

	double spotCutoff;  // angle from spotDirection that is within
				// the cone of the spotlight.  If this
				// is set to 180.0, then the light is
				// omni directional.  Otherwise, it must
				// be in the range from 0 to 90.
				// Only used when position[3] != 0

	double spotDirection[3]; // Direction the spot light is pointing.
				// Only used when spotCutoff is not 180.0
				// and position[3] != 0.
				// send this as direction

	double spotExponent;	// range 0 to 128, 0 gives uniform light dist

	int referenceFrameIsRelative; // 1 relative, 0 absolute


	// These are related to the GUI within this program
	int on;
	int showGlyph;
	int inRgbMode;
	int inPositionMode;
	int lightType;  // 0 infinite, 1 local omni, 2 local spot

	int node;       //  0 no node 
			//  1 scene, 
			//  2 ether, 
			//  3 world, 
			//  4 wand, 
			//  5 head, 
			//  6 nav
			//  7 unknown

	int currEditColor;  // 0 ambient, 1 diffuse, 2 spec, 3 all tied
	int linkAmbDiffSpec;


#define SPOT_DIR_XYZ (0)
#define SPOT_DIR_HP (1)
#define SPOT_DIR_LOOKAT (2)
	int spotDirectionType;

	double spotLookAtPt[3];


	}  LightSourceDesc;


// DATA
extern LightModelDesc LightModel;

extern LightSourceDesc LightSource[8];


// DATA
extern int CurrentLightNum;



extern void formatDouble (double d, int precision, char *result);

extern void cStrToVec (char *cStr, std::vector<std::string> &vec); 

#endif // GENERAL_GUI_S_H

