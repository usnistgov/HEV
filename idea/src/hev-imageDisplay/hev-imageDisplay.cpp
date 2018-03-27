// fltk program to display a jpeg image

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iris.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Enumerations.H>

////////////////////////////////////////////////////////////////////////
// evil global variables
bool noborder = false ;
bool fullscreen = false ;
Fl_Double_Window* window ;
Fl_Box* box ;
std::vector<Fl_Shared_Image*> imgs ;
dtkSharedMem *shm = NULL ;
int num = 0 ;
int oldNum = 0 ;
char *prog = const_cast<char*>("hev-imageDisplay") ;

////////////////////////////////////////////////////////////////////////
void usage() {
    fprintf(stderr,"Usage: %s [FLTK options] [--noborder] [--fullscreen] ] [-shm name] file [...]\n", prog) ;
}

////////////////////////////////////////////////////////////////////////
void shm_timer_callback(void*) {

    shm->read(&num) ;
    if (num<0 || num>=(int)imgs.size()) {
	fprintf(stderr, "%s: new image index %d out of range, must be between 0 and %d, inclusive\n", prog, num, (int)imgs.size()-1);
    }

    else if (num != oldNum) {
	oldNum = num ;
	box->image(imgs[num]) ;
	box->redraw() ;
	window->redraw() ;
    }

    Fl::repeat_timeout(.01, shm_timer_callback);  

}

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
static void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
  
    if (argc==1) { usage() ; return 1 ; } ;
    // initialize image lib
    fl_register_images() ;
    Fl::visual(FL_RGB);

    int fltk_argc;
    char **fltk_argv;
    int other_argc;
    char **other_argv;
    // dtkFLTKOptions_get will pull out FLTK options into its return pointer.
    if(dtkFLTKOptions_get(argc, (const char **) argv,
			  &fltk_argc,  &fltk_argv,
			  &other_argc, &other_argv)) return 1;
  
    if (other_argc==1) { usage() ; return 1 ; } ;

    // parse other args
    unsigned char r = 0, g = 0, b = 0 ;
    bool colorAssigned = false ;
    int w = 0, h = 0 ;
    bool widthSet = false ;
    bool heightSet = false ;
    int o = 1 ;
    bool noEscape = false;
    while (o<other_argc) {
	if (!strncasecmp(other_argv[o],"--noborder",5)) {
	    noborder = true ;
	} else if (!strncasecmp(other_argv[o],"--fullscreen",3)) {
	    fullscreen = true ;
	} else if (!strncasecmp(other_argv[o],"--noescape",5)) {
	    noEscape = true ;
	} else if (!strncasecmp(other_argv[o],"--color",3)) {
	    o++ ;
	    if (o == other_argc) { usage() ; return 1 ; } ;
	    r = (unsigned char)(255.f*atof(other_argv[o])) ;
	    o++ ;
	    if (o == other_argc) { usage() ; return 1 ; } ;
	    g = (unsigned char)(255.f*atof(other_argv[o])) ;
	    o++ ;
	    if (o == other_argc) { usage() ; return 1 ; } ;
	    b = (unsigned char)(255.f*atof(other_argv[o])) ;
	    colorAssigned = true ;
	} else if (!strncasecmp(other_argv[o],"--shm",3)) {
	    o++ ;
	    if (o == other_argc) { usage() ; return 1 ; } ;
	    shm = new dtkSharedMem(sizeof(int), other_argv[o]) ;
	    if (shm->isInvalid())
	    {
		fprintf(stderr, "%s: can't open shared memory file %s\n", prog, other_argv[o]);
		return 1 ;	
	    }
	    shm->read(&num) ;
	    oldNum = num ;
	} else if (!strncasecmp(other_argv[o],"--width",3)) {
	    o++ ;
	    if (o == other_argc) { usage() ; return 1 ; } ;
	    w = atoi(other_argv[o]) ;
	    if (w<=0) { usage() ; return 1 ; } ;
	    widthSet = true ;
	} else if (!strncasecmp(other_argv[o],"--height",3)) {
	    o++ ;
	    if (o == other_argc) { usage() ; return 1 ; } ;
	    h = atoi(other_argv[o]) ;
	    if (h<=0) { usage() ; return 1 ; } ;
	    heightSet = true ;
	} else {
	    break;
	}
	o++ ;
	if (!colorAssigned && o == other_argc) { usage() ; return 1 ; } ;
    }

    if (!colorAssigned && o == other_argc) { usage() ; return 1 ; } ;
    Fl_Shared_Image* img ;

    // using stdin or files on the command line?
    if (o == other_argc-1 && !strcmp(other_argv[o], "-"))
    {
	std::string line ;
	// wait for 10 seconds for stdin to time out
	while (iris::GetLine(&line,10.f))
	{
	    // each line can contain multiple file names
	    std::vector<std::string> vec = iris::ParseString(line) ;
	    for (unsigned int i=0; i<vec.size(); i++)
	    {
		struct stat buf ;
		if (stat(vec[i].c_str(), &buf)!=0)
		{
		    perror(vec[i].c_str()) ;
		    return 1 ;
		}
		
		img = Fl_Shared_Image::get(vec[i].c_str()) ;
		if (!img) {
		    fprintf(stderr, "%s: file or image file format not recognized for file %s\n", prog, vec[i].c_str());
		    return 1 ;
		}
		if (!widthSet && img->w()>w) w = img->w() ;
		if (!heightSet && img->h()>h) h = img->h() ;
		imgs.push_back(img) ;
	    }
	}
    }
    else
    {
	while (o<other_argc)
	{
	    struct stat buf ;
	    if (stat(other_argv[o], &buf)!=0)
	    {
		perror(other_argv[o]) ;
		return 1 ;
	    }
	    
	    img = Fl_Shared_Image::get(other_argv[o]) ;
	    if (!img) {
		fprintf(stderr, "%s: file or image file format not recognized for file %s\n", prog, other_argv[o]);
		return 1 ;
	    }
	    if (!widthSet && img->w()>w) w = img->w() ;
	    if (!heightSet && img->h()>h) h = img->h() ;
	    imgs.push_back(img) ;
	    o++ ;
	}
	
    }

    if (imgs.size() > 1 && shm == NULL) {
	fprintf(stderr, "%s: --shm must be given if more than one image is to be loaded\n", prog);
	return 1 ;
    }

    window = new Fl_Double_Window(0, 0, w, h, NULL); 
    window->color(fl_rgb_color(r,g,b)) ;

    if (noEscape) window->callback(doNothingCB);

    if (noborder) {
	window->border(0) ;
    }
    if (fullscreen) {
	window->fullscreen() ;
    }

    box = new Fl_Box(0, 0, w, h) ;
    box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    if (colorAssigned && imgs.size()==0)
    {
      // fprintf(stderr, "%s: color but no image suppplied, creating empty box\n", prog) ;	
    }
    else
    {
	if (num<0 || num>=(int)imgs.size()) {
	    fprintf(stderr, "%s: initial image index %d out of range, must be between 0 and %d, inclusive\n", prog, num, (int)imgs.size()-1);
	    return 1 ;
	}
	box->image(imgs[num]) ;
    }
    box->redraw() ;

    window->end();
    window->show(fltk_argc, fltk_argv);

    if (imgs.size()>1) Fl::add_timeout(.01, shm_timer_callback);

    return Fl::run() ;

}
