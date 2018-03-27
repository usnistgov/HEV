// displays text in a box, with optional scrollbars

#define MIN_HEIGHT  (1)
#define MIN_WIDTH  (1)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>

#include <iris/Utils.h>

static void usage(char *name) 
{
    fprintf(stderr,"Usage: %s [FLTK options]\n"
	    "                 [--columns N]\n"
	    "                 [--rows N]\n"
	    "                 [--scale scale]\n"
	    "                 [--dynamic] [--noescape]\n"
	    "                 [--usleep t]\n"
	    "  if columns is supplied, the widget will display at most N\n"
	    "    characters per row.  A horizontal scrollbar will be added\n"
	    "    if any line of the data has more than N characters.\n"
	    "  if rows is supplied, the widget will display at most N rows\n"
	    "    of data.  A vertical scrollbar will be added if there are\n"
	    "    more than N lines of data\n"
	    "  the default scale is 1\n\n"
	    "  if dynamic is specified data will be continuously read from stdin.\n"
	    "    dynamic requires rows and columns to be specified so the window\n"
	    "    can be correctly sized.\n"
	    "  noescape specifies the window can't be closed by pressing escape\n"
	    "    or the X in the window decoration\n"
	    "  usleep is the number of milliseconds to wait before timing out; the\n"
	    "    default is iris::GetUsleep()\n\n"
	    "  All options can be abbreviated by their first three characters,\n"
	    "  but if more letters are given, they must match exactly\n",name) ;
}

static bool running = true ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

#if 0
////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO, "hev-messageBox: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
    running = false ;
}
#endif

Fl_Double_Window *window ;

////////////////////////////////////////////////////////////////////////
// update window even if hung on a getline
void timer_callback(void*)
{
    Fl::wait(0.01) ;
    Fl::repeat_timeout(.05, timer_callback) ;

}
////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) 
{
    
    // catch signals that kill us off
    // if you put this in, you get messages on the console when launched via IRIS:
    // _DTK_ WARNING: iris::Signal signal 2 was already set, it just got clobbered
    // _DTK_ WARNING: iris::Signal signal 3 was already set, it just got clobbered
    //iris::Signal(signal_catcher);

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // for reading stdin
    std::string line ;
    
    // number of lines read
    int lines = 0 ;
    
    // max number of rows to display
    int rows =  0 ;
    
    // max number of columns to display
    int cols = 0 ;
    
    // scale factor for window size
    float scale = 1 ;
    
    // if true, use --rows and --cols to set window size, set scroll bars and display data as it comes in
    bool dynamic = false ;

    // if true, and --dynamic is set, clear the window and only draw every "rows" number of lines
    // this can be useful for data that comes in cyclic chunks of fixed number of lines, like the output of iris-readState.
    // true if disabling ESCAPE and the window manager close button
    bool noEscape = false ;

    // size of text in text box, and label size
    uchar textsize ;
    
    int argsUsed = 0 ;
    
    Fl::args(argc, argv, argsUsed) ;
    
    char *prog = argv[0] ;

    int ticks = iris::GetUsleep() ;

    while (argsUsed<argc) 
    {
	int l = strlen(argv[argsUsed]) ;
	if (l<5)
	    l=5 ;
	
	if ((argsUsed<argc) && iris::IsSubstring("--scale",argv[argsUsed],l)) 
	{
	    argsUsed++ ;
	    if (argsUsed>=argc) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: missing scale parameter\n") ;
		return 1 ;
	    } 
	    else if (!(iris::StringToFloat(argv[argsUsed], &scale)) || scale<=0.f) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: invalid scale, \"%s\"\n",argv[argsUsed]) ;
		return 1 ;
	    } 
	    else 
	    {
		argsUsed++ ;
	    }
	    
	} 
	else if ((argsUsed<argc) && iris::IsSubstring("--rows",argv[argsUsed],l)) 
	{
	    argsUsed++ ;
	    if (argsUsed>=argc) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: missing rows parameter\n") ;
		return 1 ;
	    } 
	    else if (!(iris::StringToInt(argv[argsUsed], &rows)) || rows<=0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: invalid rows parameter, \"%s\"\n",argv[argsUsed]) ;
		return 1 ;
	    } 
	    else 
	    {
		argsUsed++ ;
	    }
	    
	}
	else if ((argsUsed<argc) && iris::IsSubstring("--usleep",argv[argsUsed],l)) 
	{
	    argsUsed++ ;
	    if (argsUsed>=argc) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: missing usleep parameter\n") ;
		return 1 ;
	    } 
	    else if (!(iris::StringToInt(argv[argsUsed], &ticks)) || ticks<0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: invalid ticks parameter, \"%s\"\n",argv[argsUsed]) ;
		return 1 ;
	    } 
	    else 
	    {
		argsUsed++ ;
	    }
	    
	}
	else if ((argsUsed<argc) && iris::IsSubstring("--columns",argv[argsUsed],l))
	{
	    argsUsed++ ;
	    if (argsUsed>=argc) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: missing columns parameter\n") ;
		return 1 ;
	    } 
	    else if (!(iris::StringToInt(argv[argsUsed], &cols)) || cols<=0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: invalid columns parameter, \"%s\"\n",argv[argsUsed]) ;
		return 1 ;
	    } 
	    else 
	    {
		argsUsed++ ;
	    }
	    
	} 
	else if (iris::IsSubstring("--noescape",argv[argsUsed],l))
	{
	    noEscape = true ;
	    argsUsed++ ;
	}
	else if (iris::IsSubstring("--dynamic",argv[argsUsed],l))
	{
	    dynamic = true ;
	    argsUsed++ ;
	}
	else 
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: invalid option, \"%s\"\n",argv[argsUsed]) ;
	    return 1 ;
	}
    }
    
    if (dynamic && (rows ==0 || cols == 0))
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-messageBox: --dynamic specified, but --rows and --cols not given\n") ;
	return 1 ;
    }

    textsize = (uchar)(12*scale) ;
    
    //Fl_Double_Window *window = new Fl_Double_Window(MIN_WIDTH, MIN_HEIGHT) ;
    window = new Fl_Double_Window(MIN_WIDTH, MIN_HEIGHT) ;

    // disable killing the window using the close button or escape button
    if (noEscape) window->callback(doNothingCB);

    Fl_Browser* b = new Fl_Browser(0, 0, MIN_WIDTH, MIN_HEIGHT);
    b->textfont(FL_COURIER) ;
    b->textsize(textsize) ;
    window->end();
    
    int maxCols = 0 ;
    if (!dynamic)
    {
	while (iris::GetLine(&line,10.f))
	{
	    lines++ ;
	    //printf("%s lines = %d, rows = %d\n",line.c_str(), lines, rows) ;
	    if ((int)line.length()>maxCols) maxCols = line.length()  ;
	    b->insert((b->size())+1,line.c_str()) ;
	    if (dynamic && lines == rows) break ;
	}
	
	//~printf("rows = %d, cols = %d\n",rows, cols) ;
	//printf("%d lines read, max cols = %d\n",lines,maxCols) ;
	
	if (lines==0)
	{
	    usage(prog) ;
	    fprintf(stderr,"\nEmpty pipe.\n") ;
	    return 1 ;
	}
    }

    // if rows or cols not specified, use what we have
    if (rows==0) rows=lines ;
    if (cols==0) cols=maxCols ;
    
    // magic numbers: hopefully size of horizontal or vertical scrollbar
    float hbar = 17.f ;
    float vbar = 15.f ;
    if (!dynamic)
    {
	if (lines>rows) 
	{
	    // set vertical scrollbar if more data lines than max to be displayed
	    if (maxCols>cols) 
	    {
		// set horizontal scrollbar too if more columns than max to be displayed
		b->has_scrollbar(Fl_Browser::BOTH) ;
	    } 
	    else 
	    {
		// only set vertical scrollbar
		b->has_scrollbar(Fl_Browser::VERTICAL) ;
		hbar = 0.f ;
	    }
	} 
	else
	{
	    // don't set vertical scrollbar
	    if (maxCols>cols)
	    {
		// set horizontal scrollbar if more columns than max to be displayed
		b->has_scrollbar(Fl_Browser::HORIZONTAL) ;
		vbar = 0.f ;
	    } 
	    else 
	    {
		// no scrollbars at all
		b->has_scrollbar(0) ;
		hbar = vbar = 0.f ;
	    }
	}
    }
    else hbar = vbar = 0.f ;


    //~printf("hbar = %f, vbar = %f\n",hbar, vbar) ;
    //~printf("lines = %d, rows = %d, cols = %d\n",lines, rows, cols) ;
    
    // try to figure size of browser widget
    int h, w ;
    // more magic numbers!
    h = (int)(scale*((5.f+hbar) + 15.f*rows)) ;
    w = (int)(scale*((5.5f+vbar) + 7.05f*cols)) ;
    
    // try to figure position of browser widget
    int x, y ;
    x = (MIN_WIDTH-w)/2 ;
    if (x<0) x=0 ;
    y = (MIN_HEIGHT-h)/2 ;
    if (y<0) y=0 ;
    
    //printf("x=%d, y=%d, w=%d, h=%d\n",x, y, w, h); 
    b->resize(x, y, w, h);
    
    if (w<MIN_WIDTH) w=MIN_WIDTH ;
    if (h<MIN_HEIGHT) h=MIN_HEIGHT ;
    //~printf("window h = %d, w = %d\n",h,w) ;
    
    window->size(w, h) ;
    window->show(argc, argv);

    std::vector<std::string> buffer ;
    while (running) 
    {
	if (dynamic)
	{
	    Fl::wait(float(ticks)/1000000.f) ;
	    while (iris::GetLine(&line,0.f))
	    {
		// read stdin in chunks of "rows"
		buffer.push_back(line) ;
		lines++ ;   
		if (lines%rows == 0)
		{
		    b->clear() ;
		    for (unsigned int i=0; i<buffer.size(); i++)
		    {
			b->insert((b->size())+1,buffer[i].c_str()) ;
		    }
		    b->bottomline((b->size())) ;
		    buffer.clear() ;
		}
	    }
	}
	else Fl::wait() ;

	// if you exit the main window, stop running the application
	if (!window->shown()) running = false ;
    }
    return 0 ;
}
