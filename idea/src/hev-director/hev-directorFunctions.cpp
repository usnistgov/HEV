#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Menu_Item.H>

#include <dtk.h>
#include <iris.h>

static bool init(int argc, char** argv) ;
static void usage() ;
static void timer_callback(void*) ;

static Fl_Double_Window* main_window ;
static Fl_Double_Window* settings_window ;
static Fl_Value_Input* frameValue ;
static Fl_Roller* frameRoller ;
static Fl_Value_Input* strideValue ;
static Fl_Roller* strideRoller ;
static Fl_Value_Input* firstValue ;
static Fl_Roller* firstRoller ;
static Fl_Value_Input* lastValue ;
static Fl_Roller* lastRoller ;
static Fl_Value_Input* frameTimeValue ;
static Fl_Roller* frameTimeRoller ;
static Fl_Button* jump_to_firstButton ;
static Fl_Button* play_backwardButton ;
static Fl_Button* step_backwardButton ;
static Fl_Button* stopButton ;
static Fl_Button* step_forwardButton ;
static Fl_Button* play_forwardButton ;
static Fl_Button* jump_to_lastButton ;
static Fl_Button* swapButton ;
static Fl_Group* childrenGroup ;
static Fl_Light_Button* loopButton ;
static Fl_Light_Button* syncButton ;
static Fl_Light_Button* settingsButton ;

// separate out the fltk arguments from the others
static int fltk_argc;
static char** fltk_argv;
static int other_argc;
static char** other_argv;

static dtkSharedMem* shm = NULL ;
static unsigned int current = 0 ;
static bool forward ;
static bool playing = false ;
static bool init_loop_frames = true ;
static bool loop_frames = init_loop_frames ;
static unsigned int init_stride = 1 ;
static unsigned int stride = init_stride ;
static unsigned int init_first = 0 ;
static unsigned int first = init_first ;
static unsigned int init_last = 0 ;
static unsigned int last ;
static float init_frameTime = .1 ;
static float frameTime = init_frameTime ;
static unsigned int saved_frame ;
static bool stdout_spec = false ;
static bool use_stdout = true ;
static bool noescape = false ;
static bool settingsState = false ;

static void jump_to_first() ;
static void play_backward() ;
static void step_backward() ;
static void stop() ;
static void step_forward() ;
static void play_forward() ;
static void jump_to_last() ;
static void frame(unsigned int) ;
static void start() ;
static void init_widgets() ;
static void update_widgets() ;
static void save_frame(unsigned int) ;
static void swap_frames() ;
static void settings(bool) ;
static void reset() ;

////////////////////////////////////////////////////////////////////////
static bool init(int argc, char** argv)
{
    if (argc < 3) return false ;
    
    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    int pos ;

    if ((pos=args.findSubstring("--noescape",3))>0)
    {
        noescape = true;
        args.remove(pos) ;
        dtkMsg.add(DTKMSG_INFO, "hev-director: disabling ESCAPE and window manager close button\n");
    }

    if ((pos=args.findSubstring("--first",3))>0 && args.argc()>pos+1)
    {
	int first_frame ;
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(first_frame)) || first_frame<0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-director: illegal first frame number\n") ;
	    return false ;
	}
	first = init_first = first_frame ;
	current = first+1 ;
    }

    if ((pos=args.findSubstring("--last",3))>0 && args.argc()>pos+1)
    {
	int last_frame ;
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(last_frame)) || last_frame<=0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-director: illegal last frame number\n") ;
	    return false ;
	}
	last = init_last = last_frame ;
    }

    if ((pos=args.findSubstring("--time",3))>0 && args.argc()>pos+1)
    {
	float t ;
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(t)) || t<=0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-director: illegal frame time\n") ;
	    return false ;
	}
	frameTime = init_frameTime = t ;
    }

    if ((pos=args.findSubstring("--stdout",3))>0)
    {
        stdout_spec = true ;
        use_stdout = true ;
        args.remove(pos);
    }

    if (args.argc()==2)
    {
        shm = new dtkSharedMem(sizeof(int), args[1]) ;
        if (shm->isInvalid()) return false ;
        if (!stdout_spec) use_stdout = false ;
    }

    if (init_last == 0)
    {
        dtkMsg.add(DTKMSG_ERROR, "hev-director: --last not specified\n") ; 
        return false ;
    }
    if (init_last <= init_first)
    {
        dtkMsg.add(DTKMSG_ERROR, "hev-director: --last not greater than --first\n") ; 
        return false ;
    }

    dtkMsg.add(DTKMSG_INFO, "hev-director: init_first = %d, first = %d, init_last = %d, last = %d\n",init_first,first,init_last,last) ;

    fflush(stdout) ;
    return true ;
}

////////////////////////////////////////////////////////////////////////
static void usage()
{
    fprintf(stderr,"Usage: hev-director [ --noescape ] [ --first n ] [ --time t ] [ --stdout ] --last n [ shm ] \n") ;
}

////////////////////////////////////////////////////////////////////////
static void jump_to_first() 
{
    //fprintf(stderr," jump_to_first() \n") ;
    frame(first) ;
}

////////////////////////////////////////////////////////////////////////
static void jump_to_last() 
{
    //fprintf(stderr," jump_to_last() \n") ;
    frame(last) ;
}

////////////////////////////////////////////////////////////////////////
static void play_backward() 
{
    //fprintf(stderr," play_backward() \n") ;
    forward = false ;
    start() ;
    update_widgets() ;
}

////////////////////////////////////////////////////////////////////////
static void step_backward() 
{
    int next = (int) current - (int) stride ;
    if (loop_frames) 
    {
	if (next<(int)first)
	{
	    next = last - first + next + 1 ;
	}
	frame(next) ;
    }
    else if (next>=(int)first) frame(next) ;
    else if (playing) stop() ;
}

////////////////////////////////////////////////////////////////////////
static void stop() 
{
    //fprintf(stderr," stop() \n") ;
    playing = false ;
    update_widgets() ;
}

////////////////////////////////////////////////////////////////////////
static void step_forward() 
{
    int next = current + stride ;
    if (loop_frames) 
    {
	if (next>last) next = next - last + first - 1 ;
	frame(next) ;
    }
    else if (next<=last) frame(next) ;
    else if (playing) stop() ;
}

////////////////////////////////////////////////////////////////////////
static void play_forward() 
{
    //fprintf(stderr," play_forward() \n") ;
    forward = true ;
    start() ;
}

////////////////////////////////////////////////////////////////////////
static void frame(unsigned int f) 
{
    if (f != current)
    {
        if (use_stdout) printf("%d\n", f);
	fflush(stdout) ;
	current = f ;
	update_widgets() ;
	if (shm) shm->write(&current) ;
    }
}

////////////////////////////////////////////////////////////////////////
static void start()
{
    playing = true ;
    Fl::add_timeout(frameTime, timer_callback) ;
}
    
////////////////////////////////////////////////////////////////////////
static void timer_callback(void*)
{

    if (playing) Fl::repeat_timeout(frameTime, timer_callback) ;
    if (forward) step_forward() ;
    else step_backward() ;
    //fprintf(stderr, "timer callback\n") ;
}

////////////////////////////////////////////////////////////////////////
static void init_widgets()
{
    static std::string label = iris::IntToString(last-first+1) + " total frames" ;
    childrenGroup->label(label.c_str()) ;

    frameRoller->maximum(last) ;
    strideRoller->maximum(last) ;
    firstRoller->maximum(last) ;
    lastRoller->maximum(last) ;

    frameRoller->minimum(first) ;
    strideRoller->minimum(1) ;
    firstRoller->minimum(first) ;
    lastRoller->minimum(first) ;

    loopButton->value(init_loop_frames) ;
    //frameValue->value(first) ;

    frameValue->maximum(last) ;
    strideValue->maximum(last) ;
    firstValue->maximum(last) ;
    lastValue->maximum(last) ;

    frameValue->minimum(first) ;
    strideValue->minimum(1) ;
    firstValue->minimum(first) ;
    lastValue->minimum(first) ;

    frameTimeRoller->value(frameTime) ;
    frameTimeValue->value(frameTime) ;

    loopButton->value(loop_frames) ;

    save_frame(last) ;
}

////////////////////////////////////////////////////////////////////////
static void update_widgets()
{
    if (playing)
    {
	stopButton->activate() ;
	jump_to_firstButton->deactivate() ;
	play_backwardButton->deactivate() ;
	step_backwardButton->deactivate() ;
	step_forwardButton->deactivate() ;
	play_forwardButton->deactivate() ;
	jump_to_lastButton->deactivate() ;
    }
    else
    {
	stopButton->deactivate() ;
	jump_to_firstButton->activate() ;
	play_backwardButton->activate() ;
	step_backwardButton->activate() ;
	step_forwardButton->activate() ;
	play_forwardButton->activate() ;
	jump_to_lastButton->activate() ;
    }

    if (stride>(last-first)) stride = last-first ;

    if (current<first) frame(first) ;
    else if (current>last) frame(last) ;

    frameRoller->value((double)current) ;
    frameValue->value(current) ;
    
    strideRoller->value((double)stride) ;
    strideValue->value(stride) ;
    
    firstRoller->value((double)first) ;
    firstValue->value(first) ;
    firstRoller->maximum(last) ;
    
    lastRoller->value((double)last) ;
    lastValue->value(last) ;
    lastRoller->minimum(first) ;
    
    frameTimeRoller->value(frameTime) ;
    frameTimeValue->value(frameTime) ;

    loopButton->value(loop_frames) ;

    if (!loop_frames)
    {
	if (current == first)
	{
	    jump_to_firstButton->deactivate() ;
	    play_backwardButton->deactivate() ;
	    step_backwardButton->deactivate() ;
	}
	else if (current == last)
	{
	    step_forwardButton->deactivate() ;
	    play_forwardButton->deactivate() ;
	    jump_to_lastButton->deactivate() ;
	}
    }
}

////////////////////////////////////////////////////////////////////////
static void save_frame(unsigned int f)
{
    //fprintf(stderr, " save_frame %d\n",f) ;
    saved_frame = f ;
    static std::string label ;
    label = iris::IntToString(saved_frame) ;
    swapButton->label(label.c_str()) ;
    Fl::redraw() ;
}

////////////////////////////////////////////////////////////////////////
static void swap_frames()
{
    //fprintf(stderr, " swap_frames\n") ;
    int c = current ;
    frame(saved_frame) ;
    save_frame(c) ;
}

////////////////////////////////////////////////////////////////////////
static void settings(bool a)
{
    settingsState = a ;
    if (a)
    {
	settings_window->show(fltk_argc, fltk_argv);
    }
    else
    {
	settings_window->hide();
    }
    
}

////////////////////////////////////////////////////////////////////////
static void reset()
{
    stride = init_stride ;
    frameTime = init_frameTime ;
    loop_frames = init_loop_frames ;
    first = init_first ;
    last = init_last ;

    update_widgets() ;
}

