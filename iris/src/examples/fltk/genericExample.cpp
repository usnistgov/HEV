#include <signal.h>
#include <dtk.h>
#include <iris/Utils.h>

#include "genericExampleFunctions.h"
#include "genericExampleGUI.cxx"

static bool running = true ;

// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    
    ////////////////
    // this stuff is for command parsing

    if(dtkFLTKOptions_get(argc, (const char **) argv,
			    &fltk_argc,  &fltk_argv,
			    &other_argc, &other_argv)) 
    {
	usage() ;
	return 1 ;
    }
    
    // parse the local arguments and do other setup
    if(! init(other_argc, other_argv))
    {
	usage() ;
	return 1 ;
    }
    
    fprintf(stderr,"before creating window and running GUI\n") ;

    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_main_window() ;
    main_window->end();
    main_window->show(fltk_argc, fltk_argv);

    // disable killing the window using the close button or escape button
    main_window->callback(doNothingCB);

    // create the sub-window but don't show it- the more() function will do that
    sub_window = make_sub_window() ;
    sub_window->end();

    while (Fl::wait() && running)
    {
	// if you exit the main window, stop running the application
	if (!main_window->shown()) running = false ;

	// if you exit the sub-window, turn off the button
	// we have a separate state variable because the callback needs to
	// set its value
	if (!sub_window->shown() && moreState) 
	{
	    moreState = false ;
	    moreButton->value(false) ;
	}
    };


    ////////////////
    // here's your chance to do cleanup
    fprintf(stderr,"after GUI has exited\n") ;

    return 0 ;
}


