#include "hev-animatorFunctions.cpp"
#include "hev-animatorGUI.cxx"

static bool running = true ;

// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

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
    
    //fprintf(stderr,"before creating window and running GUI\n") ;

    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_main_window() ;
    main_window->end();
    main_window->show(fltk_argc, fltk_argv);

    // disable killing the window using the close button or escape button
    if (noescape) main_window->callback(doNothingCB);

    // create the sub-window but don't show it- the settings() function will do that
    settings_window = make_settings_window() ;
    settings_window->label("hev-animator settings") ;
    settings_window->end();

    init_widgets() ;
    jump_to_first() ;

    while (Fl::wait() && running)
    {
	// if you exit the main window, stop running the application
	if (!main_window->shown()) running = false ;

	// if you exit the sub-window, turn off the button
	// we have a separate state variable because the callback needs to
	// set its value
	if (!settings_window->shown() && settingsState) 
	{
	    settingsState = false ;
	    settingsButton->value(false) ;
	}

    };

    ////////////////
    // here's your chance to do cleanup
    //fprintf(stderr,"after GUI has exited\n") ;

    return 0 ;
}


