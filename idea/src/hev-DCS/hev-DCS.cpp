#include <iris.h>
#include "hev-DCSFunctions.cpp"
#include "hev-DCSGUI.cxx"

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
    
    // send dtkMsg files to stderr
    // to get INFO level messages set the envvar DTK_SPEW=info
    dtkMsg.setFile(stderr) ;

    // set the signls that kill us off
    iris::Signal(signal_catcher) ;
    
    ////////////////
    // this stuff is for command parsing- it pulls out the fltk arguments
    // and leaves the rest for our program
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
    main_window = make_window() ;
    main_window->end();

    printf("DCS %s\n",node.c_str()) ;
    if (under!="") printf("ADDCHILD %s %s\n",node.c_str(),under.c_str()) ;
    
    reset() ;
    update() ;
    main_window->label(node.c_str()) ;
    main_window->show(fltk_argc, fltk_argv);
    
    

    // the GUI loop
    while (running)
    {
        Fl::wait() ;
    };


    ////////////////
    // here's your chance to do cleanup
    //fprintf(stderr,"after GUI has exited\n") ;

    return 0 ;
}


