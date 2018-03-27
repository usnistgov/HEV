#include <stdio.h>
#include <sstream>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Roller.H>

#include <iris.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Light_Button.H>

#include <iris.h>

// you also need to include "hev-navigationControl.cxx just before main()
#include "hev-navigationControl.h"

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

Fl_Double_Window *main_window ;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

// fd of fifo for sending query to iris
int iris_fd ;

// fifo for reading the response from iris
std::string fifoName = std::string("/tmp/hev-navigationControl-") + std::string(getenv("USER")) ;
iris::FifoReader fifo ;

// list of navigations
std::vector<std::string> navigations ;

// generic menu entry
Fl_Menu_Item item = {NULL, 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0} ;	
Fl_Menu_Item nullitem = {NULL, 0,  0, 0, 0, 0, 0, 0, 0} ;	

// list of navigation menu items
Fl_Menu_Item* list = NULL ;

// name of current navigation
std::string currentNavName ;

// for getting the current navigation name and nav matrix
iris::ShmState shmState  ;

// scale and response values from last frame
float oldScale = 0 ;
float oldResponse = 0 ;

// how long to sleep
int ticks = iris::GetUsleep() ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
bool writeFifo(std::string line)
{
    int ret = write(iris_fd,line.c_str(),line.size()) ;
    if (ret != line.size())
    {
	dtkMsg.add(DTKMSG_WARNING,"hev-navigationControl: fifo write returns %d, tried to write %d bytes\n",ret,line.size()) ;
	return false ;
    }
    else return true ;
}

////////////////////////////////////////////////////////////////////////
void setNavigation(int i)
{
    std::string line = "NAVIGATION USE " + std::string(list[i].text) + "\n" ;
    writeFifo(line) ;
    currentNavName = list[i].text ;
    currentNav->label(currentNavName.c_str()) ;
}

////////////////////////////////////////////////////////////////////////
void setScale(float s, bool justupdateroller=false)
{
    std::string ss = iris::FloatToString(expf(s)) ;
    //fprintf(stderr, "setScale: scale from roller = %f, scaled scale = %s\n",s,ss.c_str()) ;
    if (!justupdateroller)
    {
	std::string line = "NAVIGATION PIVOTSCALE " + ss + "\n" ;
	writeFifo(line) ;
    }
    scale->value(s) ;
    scaleValue->value(("scale: " + ss).c_str()) ;
}

////////////////////////////////////////////////////////////////////////
void setResponse(float r, bool justupdateroller=false)
{
    std::string rs = iris::FloatToString(expf(r)) ;
    if (!justupdateroller)
    {
	std::string line = "NAVIGATION RESPONSE " + rs + "\n" ;
	writeFifo(line) ;
    }
    responseValue->value(("response: " + rs).c_str()) ;
}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-navigationControl [ --noescape ] [--usleep t] [--fifo fifoName]\n") ;
}

////////////////////////////////////////////////////////////////////////
void getCurrentNavName()
{
    static char* c = NULL ;
    std::map<std::string, iris::ShmState::DataElement>::iterator pos ;
    pos = shmState.getMap()->find("navName") ;
    if (pos == shmState.getMap()->end())
    {
	dtkMsg.add(DTKMSG_ERROR,"hev-navigationControl: can't get data for navName\n") ;	
    }
    else
    {
	iris::ShmState::DataElement d = pos->second ;
	if (!c) c = (char*) malloc(d.shm->getSize()) ;
	d.shm->read(c) ;
	currentNavName = c ;
	currentNav->label(c) ;
    }
}

////////////////////////////////////////////////////////////////////////
void getNavScaleAndResponse()
{
    std::map<std::string, iris::ShmState::DataElement>::iterator responsePos ;
    responsePos = shmState.getMap()->find("response") ;
    if (responsePos == shmState.getMap()->end())
    {
	dtkMsg.add(DTKMSG_ERROR,"hev-navigationControl: can't get data for response\n") ;	
    }
    else
    {
	iris::ShmState::DataElement d = responsePos->second ;
	float r ;
	d.shm->read(&r) ;
	if (r != oldResponse)
	{
	    setResponse(logf(r), true) ;
	    oldResponse = r ;
	}
    }

    std::map<std::string, iris::ShmState::DataElement>::iterator navPos ;
    navPos = shmState.getMap()->find("nav") ;
    if (navPos == shmState.getMap()->end())
    {
	dtkMsg.add(DTKMSG_ERROR,"hev-navigationControl: can't get data for nav\n") ;	
    }
    else
    {
	iris::ShmState::DataElement d = navPos->second ;
	osg::Matrix n ;
	d.shm->read(&n) ;
	float s = n.getScale().x() ; 
	if (s != oldScale)
	{
	    setScale(logf(s), true) ;
	    oldScale = s ;
	}
    }
}

////////////////////////////////////////////////////////////////////////
void setMenu()
{
    static bool first = true ;
    if (first)
    {
	dtkMsg.add(DTKMSG_INFO,"hev-navigationControl: updating navigation button\n") ;
	first = false ;
    }


    navigation->activate() ;
    navigation->show() ;
    response->show() ;
    scale->show() ;
    reset->show() ;

    if (navigations.size() == 0)
    {
	currentNav->label("NO NAVIGATIONS") ;	
	navigation->hide() ;
	response->hide() ;
	scale->hide() ;
	reset->hide() ;
    }
    else if (navigations.size() == 1)
    {
	navigation->deactivate() ;
    }

    // resize array of menu entries
    list = static_cast<Fl_Menu_Item*>(realloc(list,sizeof(Fl_Menu_Item)*(navigations.size()+1))) ;

    for (int i=0; i<navigations.size(); i++)
    {
	list[i] = item ;
	list[i].text = strdup(navigations[i].c_str()) ;
    }
    list[navigations.size()] = nullitem ;
    
    currentNavName = navigations[navigations.size()-1] ;
    currentNav->label(currentNavName.c_str()) ;
    navigation->menu(list);
}

////////////////////////////////////////////////////////////////////////
bool query()
{
    // how many navigations do we have?
    std::string query = "QUERY " + std::string(fifoName) + " NAVIGATION\n" ;
    return writeFifo(query) ;
}

////////////////////////////////////////////////////////////////////////
bool answer()
{
    // wait for answer
    std::string line ;
    while (1)
    {
	if (fifo.readLine(&line)) 
	{
	    std::vector<std::string> vec ;
	    vec = iris::ParseString(line) ;
	    if (vec.size()-2 != navigations.size())
	    {
		navigations.clear() ;
		for (int i=2; i<vec.size(); i++) navigations.push_back(vec[i]) ;
		setMenu() ;
	    }
	    return true ;
	}
	else usleep(ticks) ;
    }
    return true ;
}

////////////////////////////////////////////////////////////////////////
static void timer_callback(void*)
{
    
    Fl::repeat_timeout(.1, timer_callback) ;
    
    getCurrentNavName() ;
    getNavScaleAndResponse() ;

    // how many navigations are there?
    std::map<std::string, iris::ShmState::DataElement>::iterator pos ;
    pos = shmState.getMap()->find("numNavs") ;
    if (pos == shmState.getMap()->end())
    {
	dtkMsg.add(DTKMSG_ERROR,"hev-navigationControl: can't get data for numNavs\n") ;	
    }
    else
    {
	int num ;
	iris::ShmState::DataElement d = pos->second ;
	d.shm->read(&num) ;
	if (num != navigations.size())
	{
	    // ask about navigations
	    query() ;
	    answer() ;
	}
    }
}

////////////////////////////////////////////////////////////////////////
void setReset()
{
    Fl::remove_timeout(timer_callback) ;
    //std::string line = "NAVIGATION RESET\n" ;
    std::string line = "DSO " + currentNavName + " RESET\n" ;
    oldScale = 1.f ;
    oldResponse = 1.f ;
    writeFifo(line) ;
    scale->value(logf(oldScale)) ;
    setScale(logf(oldScale)) ;
    response->value(logf(oldResponse)) ;
    setResponse(logf(oldResponse)) ;
    Fl::add_timeout(.1, timer_callback) ;
}

////////////////////////////////////////////////////////////////////////
bool init(int argc, char **argv)
{
    if (argc > 6)
    {
	return false ;
    }

    int c = 1 ;
    while (c<argc && (argv[c][0] == '-' && argv[c][1] == '-'))
    {
	if (iris::IsSubstring("--noescape",argv[c],3))
	{
	    noEscape = true ;
	    c++ ;
	}
	else if (iris::IsSubstring("--usleep",argv[c],3))
	{
	    c++ ;
	    if (c<argc && !iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-navigationControl: invalid usleep value\n") ;
		usage() ;
		return false ; 
	    }
	    c++ ;
	}
	else if (iris::IsSubstring("--fifo",argv[c],3))
	{
	    c++ ;
	    if (c<argc) fifoName = argv[c] ;
	    else return false ;
	    c++ ;
	}
	else return false ;
    }

    // open the fifo to write to iris
    iris_fd = open(getenv("IRIS_CONTROL_FIFO"),O_WRONLY) ;
    if (iris_fd < 0)
    {
	dtkMsg.add(DTKMSG_ERROR,"hev-navigationControl: FIFO IRIS_FD = %d\n",iris_fd) ;
	return false ;
    }

    fifo.setName(fifoName) ;
    fifo.unlinkOnExit() ;
    if (!fifo.open())
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-navigationControl: can't open fifo %s\n",fifoName.c_str()) ;
	return 1 ;
    }

    std::map<std::string, iris::ShmState::DataElement>::iterator pos ;
    pos = shmState.getMap()->find("nav") ;

    Fl::add_timeout(.1, timer_callback) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////

#include "hev-navigationControl.cxx"

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
    if(!init(other_argc, other_argv))
    {
	usage() ;
	return 1 ;
    }

    ////////////////
    // start up the fltk GUI and run in a loop
    main_window = make_window() ;
    main_window->end();

    // disable killing the window using the close button or escape button
    if (noEscape) main_window->callback(doNothingCB);

    query() ; 
    answer() ; 
    main_window->show(fltk_argc, fltk_argv);

    getCurrentNavName() ;
    getNavScaleAndResponse() ;

    scale->value(logf(oldScale)) ;
    setScale(logf(oldScale),true) ;
    response->value(logf(oldResponse)) ;
    setResponse(logf(oldResponse),true) ;

    Fl::run() ;

    return 0 ;
}


