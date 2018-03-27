#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Input.H>
#include <iris.h>

class DCS
{
public:
  double xt ;
  double yt ;
  double zt ;
  double h  ;
  double p  ;
  double r  ;
  double xs ;
  double ys ;
  double zs ;
} ;

// stores the initial, current and previous roller values
DCS current ;
DCS initial ;
DCS previous ;

// stores the current and previous transformation
// for some reason, if you do something like "hRoller->value(2)" the next
// callback the value of the roller will not be 2+the change, but it's like
// it never got set to 2
DCS currentTransformation ;
DCS previousTransformation ;

// widgets
Fl_Double_Window *main_window ;
Fl_Roller *xtRoller, *ytRoller, *ztRoller, *hRoller, *pRoller, *rRoller, *xsRoller, *ysRoller, *zsRoller ;
Fl_Input *xtText, *ytText, *ztText, *hText, *pText, *rText, *xsText, *ysText, *zsText ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

std::string under ;
std::string node ;

double step = .002f ;

bool uniformScaling = true ;
bool relativeRollers = true ;

// function templates
bool init(int argc, char **argv) ;
void usage() ;
void reset() ;
void update() ;
void setUniformScaling(bool) ;
void setRelativeRollers(bool) ;

////////////////////////////////////////////////////////////////////////
void usage()
{
  fprintf(stderr,"Usage: hev-DCS [ --step value ] [ --under nodeName ] nodeName [ xt yt zt [ h p r [ xs [ ys zs ] ] ] ] ]\n") ;
}

////////////////////////////////////////////////////////////////////////
bool init(int argc, char **argv)
{

  initial.xt = 0.f ;
  initial.yt = 0.f ;
  initial.zt = 0.f ;
  initial.h  = 0.f ;
  initial.p  = 0.f ;
  initial.r  = 0.f ;
  initial.xs = 1.f ;
  initial.ys = 1.f ;
  initial.zs = 1.f ;

  reset() ;
    
  if (argc == 1) return false ;

  bool done = false ;

  int i = 1 ;

  while (!done) 
  {
    //fprintf(stderr,"i = %d, argv[%d] = %s\n",i,i,argv[i]) ;
    if (!strcasecmp("--step", argv[i]))
    {
      //fprintf(stderr,"step\n") ;
      i++ ;
      if (i<argc)
      {
        //fprintf(stderr,"  value %s\n", argv[i]) ;
        step = atof(argv[i]) ;
        i++ ;
      }
      else
      {
        return false ;
      }
    }

    else if (!strcasecmp("--under", argv[i]))
    {
      //fprintf(stderr,"under\n") ;
      i++ ;
      if (i<argc)
      {
        //fprintf(stderr,"  node %s\n", argv[i]) ;
        under = argv[i] ;
       i++ ;
      }
      else
      {
        return false ;
      }
    }

    else 
    {
      done = true ;
    }
  }

  if (argc==i) return false ; // no node name

  node = argv[i] ;
  //fprintf(stderr,"node = %s\n",argv[i]) ;
  i++ ;

  //fprintf(stderr,"i = %d, argc = %d (%d)\n",i,argc,argc-i) ;
  if (argc==i) return true ; // no more params


  if (argc-i>=3) // initial.xt initial.yt initial.zt
  {
    if ( (sscanf(argv[i],"%lf",&(initial.xt)) != 1) || 
         (sscanf(argv[i+1],"%lf",&(initial.yt)) != 1) || 
         (sscanf(argv[i+2],"%lf",&(initial.zt)) != 1) )
    {
      return false ;
    }

    //fprintf(stderr,"initial.xt initial.ytinit initial.zt = %f %f %f\n",
            //initial.xt, initial.yt, initial.zt) ;
  }

  if (argc-i>=6) // initial.h initial.p initial.r
  {
    //fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
    if ( (sscanf(argv[i+3],"%lf",&(initial.h)) != 1) || 
         (sscanf(argv[i+4],"%lf",&(initial.p)) != 1) || 
         (sscanf(argv[i+5],"%lf",&(initial.r)) != 1) )
    {
      return false ;
    }

    //fprintf(stderr,"initial.h initial.p initial.r = %f %f %f\n",
            //initial.h, initial.p, initial.r) ;
  }

  if (argc-i==7) // initial.xs
  {
    //fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
    if ( (sscanf(argv[i+6],"%lf",&(initial.xs)) != 1) )
    {
      return false ;
    }

    initial.zs = initial.ys = initial.xs ;

    //fprintf(stderr,"initial.xs = %f\n",initial.xs) ;
  }

  if (argc-i==9) // initial.xs initial.ys initial.zs
  {
    //fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
    if ( (sscanf(argv[i+6],"%lf",&(initial.xs)) != 1) || 
         (sscanf(argv[i+7],"%lf",&(initial.ys)) != 1) || 
         (sscanf(argv[i+8],"%lf",&(initial.zs)) != 1) )
    {
      return false ;
    }

    //fprintf(stderr,"initial.xs initial.ys initial.zs = %f %f %f\n",
            //initial.xs, initial.ys, initial.zs) ;
  }

  if (argc-i==8 || argc-i>9) return false ;

  //fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;

  reset() ;

  return true ;
}

////////////////////////////////////////////////////////////////////////
// update rollers and text output with world or dgl value
void update()
{
    
  if (relativeRollers)
    {
      // previous value of rollers
      //fprintf(stderr,"previous = %f %f %f  %f %f %f\n",previous.xt, previous.yt, previous.zt, previous.h, previous.p, previous.r) ;
	
      // get matrix for previous transformation
      osg::Matrix mat = iris::CoordToMatrix(previousTransformation.xt, previousTransformation.yt, previousTransformation.zt, previousTransformation.h, previousTransformation.p, previousTransformation.r) ;

      // get matrix of the orientation change from rollers or text input
      mat.postMultRotate(iris::EulerToQuat(current.h-previous.h, current.p-previous.p, current.r-previous.r)) ;

      // translation and scale are OK, just need to set orientation
      currentTransformation = current ;

      osg::Quat rot = mat.getRotate() ;
      iris::QuatToEuler(rot, &(currentTransformation.h), &(currentTransformation.p), &(currentTransformation.r)) ;

    }
  else
    {
      currentTransformation = current ;
    }

  previous = current ;
  previousTransformation = currentTransformation ;

  //fprintf(stderr,"current = %f %f %f  %f %f %f\n",current.xt, current.yt, current.zt, current.h, current.p, current.r) ;

  xtRoller->value(current.xt) ;
  ytRoller->value(current.yt) ;
  ztRoller->value(current.zt) ;
  hRoller->value(current.h) ;
  pRoller->value(current.p) ;
  rRoller->value(current.r) ;
  xsRoller->value(current.xs) ;
  ysRoller->value(current.ys) ;
  zsRoller->value(current.zs) ;

  xtText->value(iris::FloatToString((float)currentTransformation.xt).c_str()) ;
  ytText->value(iris::FloatToString((float)currentTransformation.yt).c_str()) ;
  ztText->value(iris::FloatToString((float)currentTransformation.zt).c_str()) ;
  hText->value(iris::FloatToString((float)currentTransformation.h).c_str()) ;
  pText->value(iris::FloatToString((float)currentTransformation.p).c_str()) ;
  rText->value(iris::FloatToString((float)currentTransformation.r).c_str()) ;
  xsText->value(iris::FloatToString((float)currentTransformation.xs).c_str()) ;
  ysText->value(iris::FloatToString((float)currentTransformation.ys).c_str()) ;
  zsText->value(iris::FloatToString((float)currentTransformation.zs).c_str()) ;


  printf("DCS %s %f %f %f %f %f %f %f %f %f\n",node.c_str(),
	 currentTransformation.xt, currentTransformation.yt, currentTransformation.zt, currentTransformation.h, currentTransformation.p, currentTransformation.r, currentTransformation.xs, currentTransformation.ys, currentTransformation.zs) ;

  fflush(stdout) ;
    
    
}


////////////////////////////////////////////////////////////////////////
// reset dgl to initial value
void reset()
{
  //fprintf(stderr, "reset\n") ;
  current = previous = currentTransformation = previousTransformation = initial ;
}


////////////////////////////////////////////////////////////////////////
void setUniformScaling(bool b)
{
  if (!b)
    {
      ysRoller->activate() ;
      zsRoller->activate() ;
      ysText->activate() ;
      zsText->activate() ;
    }
  else
    {
      ysRoller->deactivate() ;
      zsRoller->deactivate() ;
      ysText->deactivate() ;
      zsText->deactivate() ;
    }
  uniformScaling = b ;
}

////////////////////////////////////////////////////////////////////////
void setRelativeRollers(bool b)
{
  if (!b)
    {
      hText->activate() ;
      pText->activate() ;
      rText->activate() ;
      previous = current = currentTransformation ;
    }
  else
    {
      hText->deactivate() ;
      pText->deactivate() ;
      rText->deactivate() ;
      previousTransformation = currentTransformation = current ;
    }
  relativeRollers = b ;
  update() ;
}

