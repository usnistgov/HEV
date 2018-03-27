#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>

#include <osg/Vec3>
#include <osg/Quat>
#include <osg/Matrix>

#include <iris.h>

// widgets
Fl_Double_Window *main_window ;
Fl_Roller *xRoller, *yRoller, *zRoller, *rateRoller;
Fl_Input *xText, *yText, *zText ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// defaults can be set in options
double x = 0.0, y = 0.0, z = 1.0 ; // vector, from rollers
char *node ;

// other nasty globals
bool spin = true ; // false is wiggle
double wiggleOffset = 0.0 ;  // wiggle offset from angle
double saveAngle ;  // in wiggle mode you offset angle, but don't change actual angle
std::string xs, ys, zs ;  // x, y and z strings
osg::Quat objectAngle ;  // cumulative angle to rotate object 
osg::Quat vectorAngle ;  // angle to rotate vector into x,y,z;
std::string vectorNode ;  // name of node for vector
double rate = 0.0 ;  // rate, from roller
double angle = 0.0 ;  // angle rotated around vector
double xdef, ydef, zdef ;  // x, y, z initial values, for reset

// function templates

// parse arguments, load vector object, start timer callback
bool init(int argc, char **argv) ;
// Usage:
void usage() ;
// stop rotation- called by stop button
void stop() ;
// reset vector and angle- called by reset button
void reset() ;
// update x roller and text
void updateX(double f) ;
// update y roller and text
void updateY(double f) ;
// update z roller and text
void updateZ(double f) ;
// update rate roller
void updateRate(double f) ;
// called by show vector button to turn vector object on/off
void vectorSwitch(bool b) ;
// radio buttons call this to set rotation type
void spinSwitch(bool b) ;
// timer callback to update rotation
void timer_callback(void*) ;
// gets h,p,r from xyz, angle and rate
void getEuler() ;
// prints DCS command to rotate node
void printQDCS() ;

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-vectorRotation node [ x y z ]\n") ;
}

////////////////////////////////////////////////////////////////////////
bool init(int argc, char **argv)
{
    if (argc != 2 && argc != 5) return false ;

    node = argv[1] ;

    if (argc == 5)
    {
	if (iris::StringToDouble(argv[2], &x) && iris::StringToDouble(argv[3], &y) && iris::StringToDouble(argv[4], &z))
	{
	    xs = argv[2] ;
	    ys = argv[3] ;
	    zs = argv[4] ;
	}
	else
	{
	    usage() ;
	    return false ;
	}
    }
    else
    {
	xs = iris::DoubleToString(x) ;
	ys = iris::DoubleToString(y) ;
	zs = iris::DoubleToString(z) ;
    }
    
    xdef = x ; ydef = y ; zdef = z ;

    if (x==0.0 && y==0.0 && z==0.0)
    {
	return false ;
    }

    vectorNode = std::string(argv[0]) + iris::IntToString(getpid()) ;

    printf("QDCS %s\n",vectorNode.c_str()) ;
    printf("ADDCHILD %s %s\n",vectorNode.c_str(), node) ;
    printf("LOAD %s-object %s/idea/etc/hev-vectorRotation/data/vector.osg\n",vectorNode.c_str(), getenv("HEVROOT")) ;
    printf("ADDCHILD %s-object %s\n",vectorNode.c_str(), vectorNode.c_str()) ;
    fflush(stdout) ;
    getEuler() ;

    printQDCS() ;
    
    Fl::add_timeout(iris::GetSleep(), timer_callback) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
void timer_callback(void*)
{

    if (rate != 0)
    {
	// I just played around until the speed and angle seemed OK
	// we could make these roller or cmd line params someday

	if (spin)
	{
	    angle += rate/20.0 ;
	}
	else
	{
	    // rate determines how fast you wiggle
	    wiggleOffset+=rate/20.0 ;

	    // rotate back and forth through 30 degrees
	    angle = saveAngle + sin(wiggleOffset)*M_PI/6.0 ;
	}

	getEuler() ;
	printQDCS() ;
    }

    Fl::repeat_timeout(iris::GetSleep(), timer_callback) ;
}

////////////////////////////////////////////////////////////////////////
void vectorSwitch(bool v)
{
    if (v)
    {
	printf("NODEMASK %s ON\n",vectorNode.c_str()) ;
    }
    else
    {
	printf("NODEMASK %s OFF\n",vectorNode.c_str()) ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void spinSwitch(bool v)
{
    spin = v ;
    stop() ;
    if (!spin)
    {
	wiggleOffset = 0.0 ;
	saveAngle = angle ;
    }
}

////////////////////////////////////////////////////////////////////////
void stop()
{
    updateRate(0.0) ;
}

////////////////////////////////////////////////////////////////////////
void reset()
{
    stop() ;
    wiggleOffset = saveAngle = angle = 0 ;
    updateX(xdef) ;
    updateY(ydef) ;
    updateZ(zdef) ;
}

////////////////////////////////////////////////////////////////////////
void updateX(double f)
{
    x = f ;
    xRoller->value(f) ;
    xs = iris::DoubleToString(x).substr(0,8) ;
    xText->value(xs.c_str()) ;
    getEuler() ;
    printQDCS() ;
}

////////////////////////////////////////////////////////////////////////
void updateY(double f)
{
    y = f ;
    yRoller->value(f) ;
    ys = iris::DoubleToString(y).substr(0,8) ;
    yText->value(ys.c_str()) ;
    getEuler() ;
    printQDCS() ;
}

////////////////////////////////////////////////////////////////////////
void updateZ(double f)
{
    z = f ;
    zRoller->value(f) ;
    zs = iris::DoubleToString(z).substr(0,8) ;
    zText->value(zs.c_str()) ;
    getEuler() ;
    printQDCS() ;
}

////////////////////////////////////////////////////////////////////////
void updateRate(double f)
{
    rate = f ;
    rateRoller->value(f) ;
}

////////////////////////////////////////////////////////////////////////
void printQDCS()
{
    std::string rot ;
    rot = iris::DoubleToString(objectAngle.x()) + " " + iris::DoubleToString(objectAngle.y()) + " "  + iris::DoubleToString(objectAngle.z()) + " "  + iris::DoubleToString(objectAngle.w()) ;
    printf("QDCS %s 0 0 0 %s\n",node, rot.c_str()) ;
    rot = iris::DoubleToString(vectorAngle.x()) + " " + iris::DoubleToString(vectorAngle.y()) + " "  + iris::DoubleToString(vectorAngle.z()) + " "  + iris::DoubleToString(vectorAngle.w()) ;
    printf("QDCS %s 0 0 0 %s\n",vectorNode.c_str(), rot.c_str()) ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void getEuler()
{
#define CLOSE (.001)    

    if (x>-CLOSE && x<CLOSE && y>-CLOSE && y<CLOSE && z>-CLOSE && z<CLOSE)
    {
	//fprintf(stderr,"skipping, x = %f, y = %f, z = %f\n",x,y,z) ;
	return ;
    }

    // gets 0,0,1 vector rotated correctly into xyz
    osg::Vec3 v(x,y,z) ;
    v.normalize() ;

    // rotate aound the vector
    objectAngle = osg::Quat(angle,v) ;

    // rotate vector into xyz
    vectorAngle.makeRotate(osg::Vec3(0,0,1),v) ;

}
