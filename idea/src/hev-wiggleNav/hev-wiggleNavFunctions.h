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

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

int freq = 60 ;
int step = 0 ;

enum HPR {H, P, R} ;
HPR hpr = H ;
HPR hprNow = hpr ;

bool wiggle = false ;
bool wiggleNow = wiggle ;

double swing = 5.0 ;

double h = 0.0, p = 0.0, r = 0.0 ;

////////////////////////////////////////////////////////////////////////
// called by the signal catcher set in main()
void cleanup(int s)
{
    dtkMsg.add(DTKMSG_INFO, "hev-wiggleNav::cleanup: signal %d caught in cleanup! setting running to false\n",s) ;
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
// every program should have one
void usage()
{
    fprintf(stderr,"Usage: hev-wiggleNav [--noescape]\n") ;
}

////////////////////////////////////////////////////////////////////////
static void timer_callback(void*)
{

    step = (step+1)%freq ;
    if (step == 0 || step == freq/2)
    {
	h = p = r = 0.0 ;
	
	if (!wiggle && wiggleNow)
	{
	    printf("NAV PIVOT 0 0 0 1\n") ; 
	    fflush(stdout) ;
	}
	hprNow = hpr ;
	wiggleNow = wiggle ;
	//fprintf(stderr,"reset: wiggle = %d, hpr = %d\n", wiggle, hpr) ;
    }
    
    static double oldA = 0.0 ;
    
    if (wiggleNow)
    {
	double s = sin(2.0*M_PI*(double)step/(double(freq))) ;
	double a = s*swing ;
	double d = a - oldA ;
	oldA = a ;
	//fprintf(stderr,"wiggle: step = %d, s = %f a = %f oldA = %f, d = %f hprNow = %d\n", step, s, a, oldA, d, hprNow) ;
	if (hprNow == H) h = d ;
	else if (hprNow == P) p = d ;
	else r = d ;
	osg::Quat q = iris::EulerToQuat(h, p, r) ;
	printf("NAV PIVOT %.17g %.17g %.17g %.17g\n",q.x(), q.y(), q.z(), q.w()) ; 
	fflush(stdout) ;
    }

    Fl::repeat_timeout(1.0/freq, timer_callback) ;
}
