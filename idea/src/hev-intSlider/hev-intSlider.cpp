/*

  a fltk integer slider- when the value changes, the new value is printed to
  stdout.

  You can specify the min, max and init values.  You get an error if
  min==max, or init is outside the range of min to max.  max can be less
  than min, if you want a slider that goes the other way.  if init isn't
  specified, its the average of min and max

  hev-intSlider - based on hev-hydrationSlider by John Kelso 10/2006.

                  Renamed and man page by Steve Satterfield 5/2009.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>

#include <idea/Utils.h>

// default values
#define SLIDER_MIN (-1)
#define SLIDER_MAX (49)
#define SLIDER_INIT (-1)

// last value
static int old ;
// current value
static int i ;
// holds ascii of current value
static char c[128] ;

static float diff ;
static float minFoo = SLIDER_MIN ;
static float maxFoo = SLIDER_MAX ;
static float initFoo = SLIDER_INIT ;

static Fl_Window *window ;
static Fl_Slider *slider ;
static Fl_Output *output ;
static Fl_Button *increment ;
static Fl_Button *decrement ;

static void sliderCallback(Fl_Widget *w) {  
  if (w) {
    i = (int)(floor((slider->value()+(1.f/(diff*2.f)))*diff)+minFoo) ;
  } else {
    slider->value(((float)(i)-minFoo)/diff) ;
  }


  if (i != old) {
    sprintf(c,"%d",i) ;
    output->value(c) ;
    printf("%s\n",c) ;
    fflush(stdout) ;
    old = i ;
  }
}

static void decrementCallback(Fl_Widget *w) {
  if (maxFoo>minFoo) {
    if (i>minFoo)
      i-- ;
  } else {
    if (i<minFoo)
      i++ ;
  }
  sliderCallback(NULL) ;
}

static void incrementCallback(Fl_Widget *w) {
  if (maxFoo>minFoo) {
    if (i<maxFoo)
      i++ ;
  } else {
    if (i>maxFoo)
      i-- ;
  }
  sliderCallback(NULL) ;
}

////////////////////////////////////////////////////////////////////////
// simple wrapper function to see if a string is an integer
bool isInt(char *s, int *i) {
  char *e ;
  long int l = strtol(s, &e, 10) ;
  if (s+strlen(s) == e) {
    *i = (int) l ;
    return true ;
  } else {
    return false ;
  }
}

static void usage(char *s) {
  fprintf(stderr,"Usage: %s [--mi[nimum] i] [--ma[ximum] j] [--in[itial] k] [FLTK options]\n", s) ;
}

int main (int argc, char **argv) {
  
  int fltk_argc;
  char **fltk_argv;
  int other_argc;
  char **other_argv;

  if(! idea::fltkOptions(argc, (const char **) argv,
			     &fltk_argc,  &fltk_argv,
			     &other_argc, &other_argv)) 
    {
      usage(argv[0]) ;
      return 1 ;
    }
  
  char *prog = other_argv[0] ;

  // parse other args
  {
    int o = 1 ; // skip argv[0]
    while (o<other_argc) {
      
      int l = strlen(argv[o]) ;
      if (l<4) l=4 ;
      
      if (!strncasecmp(other_argv[o],"--minimum",l)) {
	o++ ;
	if (o<other_argc) {
	  int foo ;
	  if (!isInt(other_argv[o], &foo)) {
	    fprintf(stderr,"%s: invalid minimum, \"%s\"\n",
		    prog,other_argv[o]) ;
	    usage(prog) ;
	    return -1 ;
	  }
	  minFoo = (float) foo ;
	  initFoo = (maxFoo-minFoo)/2.f ;
	} else {
	  fprintf(stderr,"%s: missing minimum parameter\n", prog) ;
	  usage(prog) ;
	  return -1 ;
	}

      } else if (!strncasecmp(other_argv[o],"--maximum",l)) {
	o++ ;
	if (o<other_argc) {
	  int foo ;
	  if (!isInt(other_argv[o], &foo)) {
	    fprintf(stderr,"%s: invalid maximum, \"%s\"\n",
		    prog,other_argv[o]) ;
	    usage(prog) ;
	    return -1 ;
	  }
	  maxFoo = (float) foo ;
	  initFoo = (maxFoo-minFoo)/2.f ;
	} else {
	  fprintf(stderr,"%s: missing maximum parameter\n", prog) ;
	  usage(prog) ;
	  return -1 ;
	}

      } else if (!strncasecmp(other_argv[o],"--initial",l)) {
	o++ ;
	if (o<other_argc) {
	  int foo ;
	  if (!isInt(other_argv[o], &foo)) {
	    fprintf(stderr,"%s: invalid initial value, \"%s\"\n",
		    prog,other_argv[o]) ;
	    usage(prog) ;
	    return -1 ;
	  }
	  initFoo = (float) foo ;
	} else {
	  fprintf(stderr,"%s: missing initial parameter\n", prog) ;
	  usage(prog) ;
	  return -1 ;
	}

      } else {
	fprintf(stderr,"%s: invalid parameter \"%s\"\n", prog, other_argv[o]) ;
	usage(prog) ;
	return -1 ;
      }
      o++ ;
    }
  }

  if (minFoo == maxFoo) {
    fprintf(stderr,"%s: minimum equals maximum\n", prog) ;
    usage(prog) ;
    return -1 ;
    
  } else if (minFoo<maxFoo) {
    if (initFoo<minFoo || initFoo>maxFoo) {
      fprintf(stderr,"%s: initial value out of range\n", prog) ;
      usage(prog) ;
      return -1 ;
    }
    
  } else {
    if (initFoo>minFoo || initFoo<maxFoo) {
      fprintf(stderr,"%s: initial value out of range\n", prog) ;
      usage(prog) ;
      return -1 ;
    }
  }

  // how many characters will be in output window?
  unsigned int maxChars = 0 ;
  sprintf(c,"%d",(int)maxFoo) ;
  if (strlen(c)>maxChars)
    maxChars = strlen(c) ;
  sprintf(c,"%d",(int)minFoo) ;
  if (strlen(c)>maxChars)
    maxChars = strlen(c) ;
  sprintf(c,"%d",(int)initFoo) ;
  if (strlen(c)>maxChars)
    maxChars = strlen(c) ;
  
  diff = maxFoo - minFoo ;
  old = (int) initFoo ;
  i = (int) initFoo ;

  //fprintf(stderr,"minFoo = %f, maxFoo = %f, initFoo = %f\n",minFoo, maxFoo, initFoo) ;
  //fprintf(stderr,"maxChars = %d\n",maxChars) ;
  int outputWidth = 20 + (maxChars-1)*10 ;
  window = new Fl_Window(40, 5, 345+outputWidth, 25);
  
  output = new Fl_Output(5, 5, outputWidth, 25);
  sprintf(c,"%d",i) ;
  output->value(c) ;

  decrement = new Fl_Button(10+outputWidth, 5, 25, 25, "@<-");
  decrement->callback(decrementCallback);

  slider = new Fl_Slider(40+outputWidth, 5, 270, 25);
  slider->callback(sliderCallback);
  slider->type(1);

  increment = new Fl_Button(315+outputWidth, 5, 25, 25, "@->");
  increment->callback(incrementCallback);

  sliderCallback(NULL) ;

  window->end() ;
  
  window->show(fltk_argc, fltk_argv);
  return Fl::run();

}

