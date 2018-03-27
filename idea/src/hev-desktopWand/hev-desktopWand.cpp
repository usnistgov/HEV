#include <dtk.h>
#include <iris.h>

#include "hev-desktopWandFunctions.cpp"
#include "hev-desktopWandGUI.cxx"

static bool running = true ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
    running = false ;
}

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
    if(!init(other_argc, other_argv))
    {
	usage() ;
	return 1 ;
    }
    
    //fprintf(stderr,"before creating window and running GUI\n") ;

    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_window() ;

    main_window->end();

    // disable killing the window using the close button or escape button
    if (noEscape) main_window->callback(doNothingCB);

    initGUI() ;

    main_window->show(fltk_argc, fltk_argv);


    while (Fl::wait() && running)
    {
	// if you exit the main window, stop running the application
	if (!main_window->shown()) running = false ;

    };



    ////////////////
    // here's your chance to do cleanup
    //fprintf(stderr,"after GUI has exited\n") ;

    return 0 ;
}


