#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Light_Button.H>

static bool init(int argc, char **argv) ;
static void usage() ;
static void timer_callback(void*) ;
static void more(bool) ;
static void roller(float) ;

static void onOff(bool a) ;
static void go() ;

static bool onOffState = false ;
static bool moreState = false ;

static Fl_Double_Window *main_window ;
static Fl_Double_Window *sub_window ;

Fl_Light_Button *moreButton ;

// separate out the fltk arguments from the others
static int fltk_argc;
static char **fltk_argv;
static int other_argc;
static char **other_argv;

////////////////////////////////////////////////////////////////////////
static bool init(int argc, char **argv)
{
    if (argc > 2)
    {
	usage() ;
	return false ;
    }
    
    if (argc == 2)
    {
	fprintf(stderr,"optional argument %s passed\n", argv[1]) ;
    }
	
    Fl::add_timeout(1, timer_callback) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
static void usage()
{
    fprintf(stderr,"Usage: hev-genericGUIexample [ argument ]\n") ;
}

////////////////////////////////////////////////////////////////////////
static void timer_callback(void*)
{

    Fl::repeat_timeout(1, timer_callback) ;

    if (!onOffState) return ;

    fprintf(stderr, "timer callback\n") ;

}

////////////////////////////////////////////////////////////////////////
static void onOff(bool a) 
{
    if (a == onOffState) return ;
    onOffState = a ;
    fprintf(stderr, "onOffState = %d\n",onOffState) ;
}

////////////////////////////////////////////////////////////////////////
static void go()
{
    fprintf(stderr, "go!\n") ;
}

////////////////////////////////////////////////////////////////////////
static void more(bool a)
{
    moreState = a ;
    if (a)
    {
	sub_window->show(fltk_argc, fltk_argv);
    }
    else
    {
	sub_window->hide();
    }

}
    

////////////////////////////////////////////////////////////////////////
static void roller(float f) 
{
    fprintf(stderr, "roller = %f\n",f) ;

}
