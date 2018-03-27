#include <signal.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <dtk.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Light_Button.H>

#include <iris.h>

#include "hev-xyzPositionerGlobals.h"
#include "hev-xyzPositionerGUI.cxx"

static bool running = true ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;
 
////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    
    ////////////////
    // this stuff is for command parsing
    int i ;
    int ret ;
    //for (i=0; i<argc; i++) { fprintf(stderr,"argv[%d] = %s\n",i,argv[i]) ; }
    ret = Fl::args(argc, argv, i, arg) ;
    //fprintf(stderr,"ret = %d\n",ret) ;
    if (ret!=argc || (xmin>=xmax) || (ymin>=ymax) || (zmin>=zmax))
    {
	usage() ;
	exit(1) ;
    }

    setup(argc,argv) ;

    //fprintf(stderr,"before creating window and running GUI\n") ;

    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_main_window() ;

    // disable killing the window using the close button or escape button
    if (noEscape) main_window->callback(doNothingCB);

    main_window->end();

    reset() ;

    main_window->show(argc, argv);


    while (Fl::wait() && running)
    {
	// if you exit the main window, stop running the application
	if (!main_window->shown()) running = false ;

    };


    ////////////////
    // here's your chance to do cleanup
    //fprintf(stderr,"after GUI has exited\n") ;
    signal_catcher(0) ;

    return 0 ;
}


