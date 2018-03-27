#include <dtk.h>
#include <iris.h>

#include "hev-clipperControlFunctions.h"
#include "hev-clipperControlGUI.cxx"

bool running = true ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
void signal_catcher(int sig)
{
    if (sig>0)
    {
	//dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
	running = false ;
    }
    // no need to unload nodes as we don't get restarted
    //else cleanup() ;
}

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    
    // catch signals that kill us off
    iris::Signal(signal_catcher);

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

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
    
    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_main_window() ;
    main_window->end();
    main_window->show(fltk_argc, fltk_argv);

    // disable killing the window using the close button or escape button
    if (noEscape) main_window->callback(doNothingCB);

    while (Fl::wait() && running)
    {
	// if you exit the main window, stop running the application
	if (!main_window->shown()) running = false ;

    };

    ////////////////
    // here's your chance to do cleanup
    signal_catcher(-1) ;

    return 0 ;
}


