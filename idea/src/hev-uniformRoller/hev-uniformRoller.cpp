#include <signal.h>

#include <idea/Utils.h>

#include "hev-uniformRollerFunctions.cpp"
#include "hev-uniformRollerGUI.cxx"

static bool running = true ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    fprintf(stderr,"PID %d caught signal %d, starting exit sequence ...\n",
	    getpid(), sig);
    running = false ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    
    // catch signals that kill us off
    signal(SIGKILL, signal_catcher);
    signal(SIGQUIT, signal_catcher);
    signal(SIGABRT, signal_catcher);
    signal(SIGINT, signal_catcher);
    signal(SIGTERM, signal_catcher);

    char window_name[128];

    ////////////////
    // this stuff is for command parsing

    if(! idea::fltkOptions(argc, (const char **) argv,
			       &fltk_argc,  &fltk_argv,
			       &other_argc, &other_argv)) 
    {
	usage() ;
	return 1 ;
    }
    
    // parse the local arguments and do other setup
    if( ! init(other_argc, other_argv) )
    {
	usage() ;
	return 1 ;
    }
    
    //fprintf(stderr,"before creating window and running GUI\n") ;

    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_window();

    sprintf( window_name, "%s::%s", node, uniform );
    printf( "%s\n", window_name );
    main_window->label( window_name );
    main_window->end();
    main_window->show(fltk_argc, fltk_argv);
    
    reset(); // Make sure roller is proper scaled for init values.

    while (Fl::wait() && running) { };

    ////////////////
    // here's your chance to do cleanup
    //fprintf(stderr,"after GUI has exited\n") ;

    return 0 ;
}


