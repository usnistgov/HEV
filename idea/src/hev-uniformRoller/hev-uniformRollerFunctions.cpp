#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>

#include <osg/Vec3>
#include <osg/Quat>
#include <osg/Matrix>

#include <dtk.h>

// Widgets
Fl_Double_Window *main_window;
Fl_Roller *xRoller;
Fl_Input *valText, *minText, *maxText;

// Separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// Defaults for roller setting to use in absence of cmd line values
float xval = 0.f, xmin = 0.f, xmax = 1.f;
const char *node = "world";
const char *uniform = "";

// Other nasty globals
char xval_str[64], xmin_str[64], xmax_str[64]; // String ver of roller settings
float xval_ini, xmin_ini, xmax_ini;            // Initial values, for reset

// Function templates
bool init(int argc, char **argv);              // Parse arguments
void usage();                                  // Usage
void reset();                                  // Reset values - called by reset button
void updateVal(float f);                       // Update roller value and text
void updateMin(float f);                       // Update roller minimum and text
void updateMax(float f);                       // Update roller maximum and text
void printSGE();                               // Prints SGE command(s) to stdout

////////////////////////////////////////////////////////////////////////
void usage()
{
  fprintf(stderr,"Usage: hev-unifromRoller node uniform [val min max]\n");
}

////////////////////////////////////////////////////////////////////////
bool init(int argc, char **argv)
{
  // Do ONLY if argc is 3 OR 6, as in:
  //    dtk-roller node uniform
  //    dtk-roller node uniform val min max
  if (argc != 3 && argc != 6) 
  {
    return false;
  }

  // Get node name from 1st arg.
  node = argv[1];
  uniform = argv[2];

  // If other args are given, get initial val, min & max values.
  // Otherwise defaults spec'd in global decl are used.
  if (argc == 6)
  {
    xval = atof(argv[3]);
    xmin = atof(argv[4]);
    xmax = atof(argv[5]);
  }

  // Clamp roller roller value min < val < max.
  if( xval < xmin )
  {
    xval = xmin;
  }
  if( xval > xmax )
  {
    xval = xmax;
  }

  // Print the float values into string vars
  sprintf(xval_str,"%6.3f",xval);
  sprintf(xmin_str,"%6.3f",xmin);
  sprintf(xmax_str,"%6.3f",xmax);

  // Set reset values (ie defaults) to used
  // by the reset button callback.
  xval_ini = xval;
  xmin_ini = xmin;
  xmax_ini = xmax;

  // Fatpoints shader for node

  // printf( "LOAD fatpointSize fatpoint.osg | hev-sgeWriteFifo\n" );
  // printf( "MERGESTATE fatpointSize %s | hev-sgeWriteFifo\n", node );
  // printf( "UNLOAD fatpointSize  | hev-sgeWriteFifo\n" );
  // system( "hev-applyState -s fatpoint.osg world\n" );
  // fflush( stdout );


  printSGE();

  return true ;
}

////////////////////////////////////////////////////////////////////////
void reset()
{
  updateMin( xmin_ini );
  updateMax( xmax_ini );
  updateVal( xval_ini );
}

////////////////////////////////////////////////////////////////////////
void updateVal(float f)
{
  xval = f;                         // Set global var to new value.

  // Clamp roller roller value min < val < max.
  if( xval < xmin )
  {
    xval = xmin;
  }
  if( xval > xmax )
  {
    xval = xmax;
  }

  sprintf( xval_str,"%6.3f",xval ); // Copy into atring for text widget.
  valText->value( xval_str );       // Set text widget to new value.
  xRoller->value( f ) ;             // Set roller value.
  printSGE();                       // Print SGE command w/ new value
}

////////////////////////////////////////////////////////////////////////
void updateMin(float f)
{
  // Make sure new min is not bigger than max.
  if( f > xmax )
  {
    f = xmax;
  }
  xmin = f;                         // Set global var to new value.
  sprintf( xmin_str,"%6.3f",xmin ); // Copy into atring for text widget.
  minText->value( xmin_str );       // Set min text widget to new value.
  xRoller->minimum( xmin );         // Set roller minimum.
  updateVal( xval );                // Update value to insure it is clamped.
}

////////////////////////////////////////////////////////////////////////
void updateMax(float f)
{
  // Make sure new max is nor smaller than min.
  if( f < xmin )
  {
      f = xmin;
  }
  xmax = f;                         // Set global var to new value.
  sprintf( xmax_str,"%6.3f",xmax ); // Copy into atring for text widget.
  maxText->value( xmax_str );       // Set max text widget to new value.
  xRoller->maximum( xmax );         // Set roller miaximum.
  updateVal( xval );                // Update value to insure it is clamped
}


////////////////////////////////////////////////////////////////////////
void printSGE()  // In fact this is a IRIS command. SGE is leftover from prev version
{
  printf( "UNIFORM %s %s %f\n", node, uniform, xval ) ;
  fflush( stdout ) ;
}

