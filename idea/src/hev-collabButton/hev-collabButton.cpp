#include <iris.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Light_Button.H>

#define COUNT (0)
#define VALUE (1)

////////////////////////////////////////////////////////////////////////
// set to false to exit
bool running = true ;
unsigned int oldCount = 0 ;

////////////////////////////////////////////////////////////////////////
class collab
{
public:
    collab(const std::string& shmName, bool readShm): _valid(false)
    {
	_shmData[COUNT] = _shmData[VALUE] = 0 ;
	if (!readShm)
	{
	    // get and initialize shared memory
	    _shm = new dtkSharedMem(sizeof(_shmData), shmName.c_str()) ;
	    if (_shm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR,"hev-collabButton: can't open shared memory file %s\n", shmName.c_str());
		return ;
	    }
	    if (_shm->write(_shmData))
	    {
		return ;
	    }
	}
	else
	{
	    // does the shared memory already exist?
	    // if it already exists read in the data
	    // if not, initialize the data to zero
	    _shm = new dtkSharedMem(sizeof(_shmData), shmName.c_str(), _shmData) ; 
	    if (_shm->isInvalid())  
	    {
		dtkMsg.add(DTKMSG_ERROR,"hev-collabButton: can't open shared memory file %s\n", shmName.c_str());
		return ;
	    }
	    oldCount = _shmData[COUNT] ;
	}
	
	_valid = true ;
	
    } ;

    bool isValid() { return _valid ; } ;

    bool isInvalid() { return !_valid ; } ;

    // write to shared memory and bump the write counter
    bool write(unsigned int i) 
    {
	_shmData[COUNT]++ ;
	_shmData[VALUE] = i ;
	if (_shm->write(_shmData))
	{
	    _valid = false ;
	    return false ;
	}
	else return true ;
    }

    // read from shared memory
    bool read(unsigned int* i, unsigned int* count=NULL) 
    {
	unsigned int shmData[2] ;
	if (_shm->read(shmData))
	{
	    _valid = false ;
	    return false ;
	}
	else
	{
	    if (i) *i = shmData[VALUE] ; 
	    if (count) *count = shmData[COUNT] ;
	    return true ;
	}
    }
    
    // set the message to be printed for button value i
    void setMessage(unsigned int i, std::string s)
    {
	_messages.insert(std::make_pair(i,s)) ;
    }

    // get the message to be printed for button value i
    bool getMessage(unsigned int i, std::string* s=NULL)
    {
	std::map<unsigned int, std::string>::iterator pos ;
	pos = _messages.find(i) ;
	if (pos == _messages.end()) return false ;
	else 
	{
	    if (s) *s = pos->second ;
	    return true ;
	}
    }
    
    // print message i
    bool doMessage(unsigned int i)
    {
	std::string s ;
	if (getMessage(i, &s))
	{
	    if (s.size() > 0)
	    {
		printf("%s\n",s.c_str()) ;
		fflush(stdout) ;
	    }
	    return true ;
	}
	else return false ;
    }

private:
    dtkSharedMem* _shm ;
    unsigned int _shmData[2] ;
    bool _valid ;
    std::map<unsigned int, std::string> _messages ;
} ;


////////////////////////////////////////////////////////////////////////
// this class combines the collab class above and the Fl_Light_Button class
class collabButton: public Fl_Light_Button, public collab
{
public:
    collabButton(int x, int y, int w, int h, const std::string& shmName, bool readShm, const char* l=0): 
	Fl_Light_Button(x, y, w, h, l), 
	collab(shmName, readShm) 
    { 
	; 
    }
} ;

////////////////////////////////////////////////////////////////////////
// this gets called whenever the FLTK button gets pressed
void buttonCB(Fl_Widget* o) 
{ 
    collabButton *cb = static_cast<collabButton*>(o) ;
    dtkMsg.add(DTKMSG_DEBUG,"hev-collabButton: buttonCB: value = %d\n",cb->value()) ;
    // update shared memory based on the new value of the button
    cb->write(cb->value()) ;
} ;

////////////////////////////////////////////////////////////////////////
// this gets called to monitor shared memory and update the FLTK widget
// the shared memory could be set externally or by pressing the button
void shm_timer_callback(void* p=NULL) 
{
    collabButton *cb = static_cast<collabButton*>(p) ;
    unsigned int i = 0 ;
    unsigned int count = 0 ;
    cb->read(&i, &count) ;
    if (count != oldCount) 
    {
	dtkMsg.add(DTKMSG_DEBUG,"hev-collabButton: old count = %d, new count = %d\n",oldCount, count) ;
	dtkMsg.add(DTKMSG_DEBUG,"hev-collabButton: new value = %d\n",i) ;
	oldCount = count ;
	if (cb->doMessage(i)) cb->value(i) ;
    }
    Fl::repeat_timeout(.01, shm_timer_callback, p);  
}

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_DEBUG,"hev-collabButton: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
    running = false ;
    iris::Signal(SIG_DFL);
}

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
void usage() 
{
    fprintf(stderr,"Usage: hev-collabButon [--noescape] [--label l] [--readShm] [--shmName n] [--height h] [--width w] [--onString \"s\"] [--offString \"s\"]\n") ;
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // catch signals that kill us off
    iris::Signal(signal_catcher);

    // parse arguments
    int fltk_argc;
    char **fltk_argv;
    int other_argc;
    char **other_argv;
    if(dtkFLTKOptions_get(argc, (const char **) argv,
			  &fltk_argc,  &fltk_argv,
			  &other_argc, &other_argv)) return 1;
    
    // options that can be set on the command line
    bool noEscape = false ;
    std::string label("collab") ;
    std::string shmName("idea/hev-collabButton") ;
    unsigned int width = 50 ;
    unsigned int height = 20 ;
    // if false, create and zero shared memory, if true create and zero if
    // it doesn't exist, but if it does exist read in the values
    bool readShm = false ;
    std::string onString ;
    std::string offString ;
    
    int c = 1 ;
    while (c<other_argc && (other_argv[c][0] == '-' && other_argv[c][1] == '-'))
    {
	if (iris::IsSubstring("--noescape",other_argv[c],3))
	{
	    noEscape = true ;
	    c++ ;
	}
	else if (iris::IsSubstring("--readShm",other_argv[c],3))
	{
	    readShm = true ;
	    c++ ;
	}
	else if (iris::IsSubstring("--label",other_argv[c],3))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no label given!\n") ;
		usage() ;
		return 1 ;
	    }
	    label = other_argv[c] ;
	    c++ ;
	}
	else if (iris::IsSubstring("--onString",other_argv[c],4))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no onString given!\n") ;
		usage() ;
		return 1 ;
	    }
	    onString = other_argv[c] ;
	    c++ ;
	}
	else if (iris::IsSubstring("--offString",other_argv[c],4))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no offString given!\n") ;
		usage() ;
		return 1 ;
	    }
	    offString = other_argv[c] ;
	    c++ ;
	}
	else if (iris::IsSubstring("--shmName",other_argv[c],3))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no shared memory name given!\n") ;
		usage() ;
		return 1 ;
	    }
	    shmName = other_argv[c] ;
	    c++ ;
	}
	else if (iris::IsSubstring("--height",other_argv[c],3))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no height value given!\n") ;
		usage() ;
		return 1 ;
	    }
	    if (!iris::StringToUInt(other_argv[c],&height))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid height value given!\n") ;
		usage() ;
		return 1 ;
	    }
	    c++ ;
	}
	else if (iris::IsSubstring("--width",other_argv[c],3))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no width value given!\n") ;
		usage() ;
		return 1 ;
	    }
	    if (!iris::StringToUInt(other_argv[c],&width))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid width value given!\n") ;
		usage() ;
		return 1 ;
	    }
	    c++ ;
	}
	else
	{
	    usage() ;
	    return 1 ;
	}
    }

    Fl_Double_Window* w = new Fl_Double_Window(10, 10, width+10, height+10, NULL);
    if (noEscape) w->callback(doNothingCB);


    collabButton* cb = new collabButton(5, 5, width, height, shmName.c_str(), readShm, label.c_str()) ;
    if (cb->isInvalid()) return 1 ;
    if (readShm)
    {
	unsigned int i ;
	cb->read(&i) ;
	cb->value(i) ;
    }
    cb->callback(buttonCB) ;
    cb->setMessage(0,offString) ;
    cb->setMessage(1,onString) ;
    w->end();
    w->show(fltk_argc, fltk_argv);

    // this read shared memory, looking for changes
    Fl::add_timeout(.01, shm_timer_callback, cb);

    while (running)
    {
	if (Fl::wait() == 0)  break;
    }
    dtkMsg.add(DTKMSG_DEBUG,"hev-collabButton: cleanup\n") ;

    return 0 ;
}
