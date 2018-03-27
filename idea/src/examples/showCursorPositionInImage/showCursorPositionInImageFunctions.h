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

// determines if we hide/show the image window
bool imageState = false ;

// image file to load and its size and name
Fl_Shared_Image* flImage ;
int h, w ;
char* image_name ;

////////////////////////////////////////////////////////////////////////
// called by the signal catcher set in main()
void cleanup(int s)
{
    dtkMsg.add(DTKMSG_INFO, "showCursorPositionInImage::cleanup: signal %d caught in cleanup! setting running to false\n",s) ;
    running = false ;
}

////////////////////////////////////////////////////////////////////////
// see if we can load the file, and if so, sniff it
bool init(int argc, char **argv)
{
    if (argc != 2) return false ;
    
    image_name = argv[1] ;
    fl_register_images();
    flImage = Fl_Shared_Image::get(image_name) ;
    if (!flImage) return false ;

    h = flImage->h() ;
    w = flImage->w() ;
    dtkMsg.add(DTKMSG_INFO, "showCursorPositionInImage::init: image %s, height = %d, width = %d\n",image_name,h,w) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
// every program should have one
void usage()
{
    fprintf(stderr,"Usage: showCursorPositionInImage image\n") ;
}

////////////////////////////////////////////////////////////////////////
// hide/show the image window
void image(bool a)
{
    imageState = a ;
    if (a)
    {
	sub_window->show(fltk_argc, fltk_argv);
    }
    else
    {
	sub_window->hide();
    }

}
