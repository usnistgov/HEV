#include <FL/Fl_Shared_Image.H>

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// if anything change it to false when it's time to exit
bool running = true ;

// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;

// for getting panes from iris
static std::vector<std::string> panes ;
static std::string paneList ;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

// true if recording
bool recording = false ;

////////////////////////////////////////////////////////////////////////
// called by the signal catcher set in main()
void cleanup(int s)
{
    dtkMsg.add(DTKMSG_INFO, "hev-frameGrabberControl::cleanup: signal %d caught in cleanup! setting running to false\n",s) ;
    running = false ;
}

////////////////////////////////////////////////////////////////////////
// parse arguments, etc.
bool init(int argc, char **argv)
{

    if (argc == 1) return true ;
    else if (argc ==2 && iris::IsSubstring("--noescape",argv[1],3))
    {
	noEscape = true ;
	return true ;
    }
    else return false ;

}

////////////////////////////////////////////////////////////////////////
void queryPanes() 
{
    // get list of pane names
    // this only needs to happen once, at startup
    std::string panesFifoName("/tmp/hev-frameGrabberControl-"+std::string(getenv("USER"))+"-panesFifo") ;
    iris::FifoReader panesFifo(panesFifoName) ;
    panesFifo.open() ;
    panesFifo.unlinkOnExit() ;
    printf("QUERY %s PANES\n",panesFifoName.c_str()) ;
    fflush(stdout) ;
    std::string line ;
    while (1)
    {
	if (panesFifo.readLine(&line))
	{
	    panes = iris::ParseString(line) ;
	    panes.erase(panes.begin()) ;
	    panes.erase(panes.begin()) ;
	    break ;
	}
	usleep(iris::GetUsleep()) ;
    }
    Fl_Menu_Item* mi = new Fl_Menu_Item[panes.size()+1];
    for (unsigned int i=0; i<panes.size(); i++) 
    {
	memset(&(mi[i]), 0, sizeof(Fl_Menu_Item)) ;
	mi[i].label(panes[i].c_str()) ;
	mi[i].flags = FL_MENU_TOGGLE ;
	if (panes[i] == "perspective") 
	{
	    mi[i].set() ;
	    paneList = panes[i] ;
	}
    }
    // do a zero'd one at the end so FLTK will know it's the end of the array
    memset(&(mi[panes.size()]), 0, sizeof(Fl_Menu_Item)) ;
    paneMenu->menu(mi) ;

    if (paneList == "")
    {
	dtkMsg.add(DTKMSG_WARNING, "hev-frameGrabberControl: no panes selected\n") ;
	recordButton->deactivate() ;
	snapButton->deactivate() ;
    }
    else
    {
	recordButton->activate() ;
	snapButton->activate() ;
    }
    return ;
}

////////////////////////////////////////////////////////////////////////
//
void selectPanes(Fl_Menu_Button* mb)
{
    // the value of the button is which item got selected
    int v = mb->value() ;

    const Fl_Menu_Item* mi = mb->menu() ;

    paneList.clear() ;
    for (unsigned int i=0; i<panes.size(); i++)
    {
	if (mi[i].value() != 0) paneList += panes[i] + " " ;
    }
    
    if (paneList == "")
    {
	dtkMsg.add(DTKMSG_WARNING, "hev-frameGrabberControl: no panes selected\n") ;
	recordButton->deactivate() ;
	snapButton->deactivate() ;
    }
    else
    {
	recordButton->activate() ;
	snapButton->activate() ;
    }
    fprintf(stderr,"selected panes: %s\n",paneList.c_str()) ;
}

////////////////////////////////////////////////////////////////////////
// every program should have one
void usage()
{
    fprintf(stderr,"Usage: hev-frameGrabberControl [--noescape]\n") ;
}

////////////////////////////////////////////////////////////////////////
// called when record button pressed
void record(bool b)
{
    if (b == recording) return ;

    if (b) printf("DSO frameGrabber START %s\n",paneList.c_str()) ;
    else printf("DSO frameGrabber STOP %s\n",paneList.c_str()) ;
    fflush(stdout) ;
    recording = b ;
}

////////////////////////////////////////////////////////////////////////
// called when snap button pressed
void snap()
{
    printf("DSO frameGrabber SNAP %s\n",paneList.c_str()) ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
// called when write button pressed
void write()
{
    printf("DSO frameGrabber FLUSH %s\n",paneList.c_str()) ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
// called when erase button pressed
void erase()
{
    printf("DSO frameGrabber RESET %s\n",paneList.c_str()) ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
// called when every valuator changed pressed
void every(float f)
{
    printf("DSO frameGrabber FRAMERATE %f\n",f) ;
    fflush(stdout) ;
}

