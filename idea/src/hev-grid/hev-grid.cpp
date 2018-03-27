#include <signal.h>
#include <string>
#include <dtk.h>
#include <osgDB/Registry>
#include <iris.h>
#include "guiWindow.h"

// wand translation can be put in this node
const std::string wandDCSname = "hev-wandDCS" ;

// grid scale can be put in this node
const std::string gridDCSname = "hev-gridDCS" ;

// nearest translation can be put in this node
const std::string nearestDCSname = "hev-nearestDCS" ;

// this object turns on and off when within proximity of granularity
std::string nearestObjectModelName  ;
const std::string nearestObjectNodeName = "hev-nearestObjectNodeName" ;

// this object just follows the wand, but always at nearest granularity
std::string proximityObjectModelName ;
const std::string proximityObjectNodeName = "hev-proximityObjectNodeName" ;

// this object just follows the wand
std::string wandObjectModelName ;
const std::string wandObjectNodeName = "hev-wandObjectNodeName" ;

// this object just follows the wand
std::string cubeObjectModelName ;
const std::string cubeObjectNodeName = "hev-cubeObjectNodeName" ;

// this object just follows the wand
std::string axesObjectModelName ;
const std::string axesObjectNodeName = "hev-axesObjectNodeName" ;

// this object is just loaded to provide a background
std::string backgroundObjectModelName = "hev-backgroundObject.osg" ;
const std::string backgroundObjectNodeName = "hev-backgroundObjectNodeName" ;


const double origProximity = .02f ;
const double origGranularity = .1f ;

// evil global variables
bool running = true ;
std::string wandShmName = "wandMatrix" ;
dtkSharedMem *wandShm ;
osg::Matrix wand ;
double wandCoord[6] ;
double oldWandCoord[6] ;

double proximity = origProximity ;
double granularity = origGranularity ;
double seconds = .01 ;
bool zeroHPR = true ;

int nearestObjectVisible = false ;
int showNearest = false ;
int showProximity = false ;
int showPicker = false ;
int showCube = false ;
int showAxes = false ;
int showBackground = false ;
int oldShowNearest ;
int oldShowProximity ;
int oldShowPicker ;
int oldShowCube ;
int oldShowAxes ;
int oldShowBackground ;

double gridScale = 1.f ;
Fl_Roller *gridScaleRoller ;
Fl_Value_Output *gridScaleOutput ;

void writeStingAndFlush(std::string line)
{
    printf("%s\n",line.c_str()) ;
    fflush(stdout) ;
}

void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO,
	       "PID %d caught signal %d, starting exit sequence ...\n",
	       getpid(), sig);
    running = false ;
}

void writeIRIS(void)
{

    std::string l("DCS " + wandDCSname + " " +
		  iris::DoubleToString(wandCoord[0]) + " " + 
		  iris::DoubleToString(wandCoord[1]) + " " + 
		  iris::DoubleToString(wandCoord[2]) + " " + 
		  iris::DoubleToString(wandCoord[3]) + " " + 
		  iris::DoubleToString(wandCoord[4]) + " " + 
		  iris::DoubleToString(wandCoord[5])) ;

    writeStingAndFlush(l) ;

    static double oldNearest[3] = {FLT_MAX, FLT_MAX, FLT_MAX} ;
    static double oldGridScale = 1 ;

    double gridScaledWand[3] ;
    gridScaledWand[0] = wandCoord[0]/gridScale ;
    gridScaledWand[1] = wandCoord[1]/gridScale ;
    gridScaledWand[2] = wandCoord[2]/gridScale ;

    double nearest[3] ;
    nearest[0] = round(gridScaledWand[0]/granularity)*granularity ;
    nearest[1] = round(gridScaledWand[1]/granularity)*granularity ;
    nearest[2] = round(gridScaledWand[2]/granularity)*granularity ;
    
    double delta[3] ;
    delta[0] = fabs(nearest[0] - gridScaledWand[0]) ;
    delta[1] = fabs(nearest[1] - gridScaledWand[1]) ;
    delta[2] = fabs(nearest[2] - gridScaledWand[2]) ;
    
    if (gridScale != oldGridScale || 
	memcmp(oldNearest, nearest, sizeof(nearest)) ||
	(memcmp(&(wandCoord[3]), &(oldWandCoord[3]), sizeof(double)*3)))
    {
	memcpy(oldNearest, nearest, sizeof(nearest)) ;
	oldGridScale = gridScale ;
	//fprintf(stderr,"DCS %f %f %f\n",nearest[0], nearest[1], nearest[2]) ;
	std::string l("DCS " + nearestDCSname + " " +
		      iris::DoubleToString(nearest[0]) + " " + 
		      iris::DoubleToString(nearest[1]) + " " + 
		      iris::DoubleToString(nearest[2]) + " " + 
		      iris::DoubleToString(wandCoord[3]) + " " + 
		      iris::DoubleToString(wandCoord[4]) + " " + 
		      iris::DoubleToString(wandCoord[5])) ;

	writeStingAndFlush(l) ;
	
    }

    if (showNearest)
    {
	if (delta[0]<proximity && delta[1]<proximity && delta[2]<proximity)
	{
	    if (!nearestObjectVisible)
	    {
		//fprintf(stderr,"ON\n") ;
		writeStingAndFlush("ADDCHILD " + nearestObjectNodeName + " " + nearestDCSname) ;
		nearestObjectVisible = true ;
	    }
	}
	else if (nearestObjectVisible)
	{
	    //fprintf(stderr,"OFF\n") ;
	    writeStingAndFlush("REMOVECHILD " + nearestObjectNodeName + " " + nearestDCSname) ;
	    nearestObjectVisible = false ;
	    
	}
    }
}

void updateButtons()
{

    if (showBackground != oldShowBackground)
    {
	if (showBackground)
	{
	    writeStingAndFlush("ADDCHILD " + backgroundObjectNodeName + " " + nearestDCSname) ;
	}
	else
	{
	    writeStingAndFlush("REMOVECHILD " + backgroundObjectNodeName + " " + nearestDCSname) ;
	}
	oldShowBackground = showBackground ;
    }

    if (showProximity != oldShowProximity)
    {
	if (showProximity)
	{
	    writeStingAndFlush("ADDCHILD " + proximityObjectNodeName + " " + nearestDCSname) ;
	}
	else
	{
	    writeStingAndFlush("REMOVECHILD " + proximityObjectNodeName + " " + nearestDCSname) ;
	}
	oldShowProximity = showProximity ;
    }

    if (showPicker != oldShowPicker)
    {
	if (showPicker)
	{
	    writeStingAndFlush("ADDCHILD " + wandObjectNodeName + " " + wandDCSname) ;
	}
	else
	{
	    writeStingAndFlush("REMOVECHILD " + wandObjectNodeName + " " + wandDCSname) ;
	}
	oldShowPicker = showPicker ;
    }

    if (showCube != oldShowCube)
    {
	if (showCube)
	{
	    writeStingAndFlush("ADDCHILD " + cubeObjectNodeName + " " + nearestDCSname) ;
	}
	else
	{
	    writeStingAndFlush("REMOVECHILD " + cubeObjectNodeName + " " + nearestDCSname) ;
	}
	oldShowCube = showCube ;
    }

    if (showAxes != oldShowAxes)
    {
	if (showAxes)
	{
	    writeStingAndFlush("ADDCHILD " + axesObjectNodeName + " " + nearestDCSname) ;
	}
	else
	{
	    writeStingAndFlush("REMOVECHILD " + axesObjectNodeName + " " + nearestDCSname) ;
	}
	oldShowAxes = showAxes ;
    }

    if (oldShowNearest != showNearest)
    {
	if (showNearest)
	{
	    //fprintf(stderr,"turning on nearest\n") ;
	    nearestObjectVisible = false ;  
	}
	else
	{
	    if (nearestObjectVisible)
	    {
		//fprintf(stderr,"turning off nearest\n") ;
		writeStingAndFlush("REMOVECHILD " + nearestObjectNodeName + " " + nearestDCSname) ;
	    }
	}
	oldShowNearest = showNearest ;
    }

    //fprintf(stderr, "calling writeIRIS() from updateButtons\n") ;
    writeIRIS() ;
}

double getGridScale(double s)
{
    return pow(2.f, s) ;  
}

void updateGridScale()
{
    gridScaleRoller->label(" ") ;
    gridScaleOutput->value(gridScale) ;

    //fprintf(stderr,"gridScale = %f\n",gridScale) ;
    
    proximity = origProximity * gridScale ;
    granularity = origGranularity * gridScale ;
    writeStingAndFlush("DCS " + gridDCSname + " 0 0 0 0 0 0 " + iris::DoubleToString(gridScale) + " " + iris::DoubleToString(gridScale) + " " + iris::DoubleToString(gridScale)) ;
    //fprintf(stderr, "calling writeIRIS() from updateGridScale\n") ;
    writeIRIS() ;
}

void timeoutCallback(void*)
{
    Fl::repeat_timeout(.1, timeoutCallback) ;

    wandShm->read(wand.ptr()) ;

    iris::MatrixToCoord(wand, wandCoord, wandCoord+1, wandCoord+2, wandCoord+3, wandCoord+4, wandCoord+5) ;
    if (zeroHPR)
    {
	wandCoord[3] = wandCoord[4] = wandCoord[5] = 0.0 ;
    }

    // only do something if things change
    if (memcmp(wandCoord, oldWandCoord, sizeof(wandCoord)))
    {
	writeIRIS() ;
	memcpy(oldWandCoord, wandCoord, sizeof(wandCoord)) ;
    }

}

////////////////////////////////////////////////////////////////////////
// argument parser callback
int arg(int argc, char **argv, int &i)
{
    if (argc >= 2) //shm
    {
	wandShmName = argv[1] ;
	fprintf(stderr,"setting wand shared memory to \"%s\"\n",wandShmName.c_str()) ;
    }

    i += argc-1 ;
    return argc-1 ;
}
 
////////////////////////////////////////////////////////////////////////
//
void usage()
{
    fprintf(stderr,"Usage: hev-grid [ shm ]\n") ;
}

////////////////////////////////////////////////////////////////////////
#include "guiWindow.cxx"


int main (int argc, char** argv)
{

    int i ;
    if (!Fl::args(argc, argv, i, arg) || (i!=1) && (i!=2))
    {
	usage() ;
	exit(1) ;
    }

    
    if (getenv("HEV_GRID_NEAREST_OBJECT"))
    {
	nearestObjectModelName = getenv("HEV_GRID_NEAREST_OBJECT") ;	
    }
    else
    {
	nearestObjectModelName = std::string(getenv("HEVROOT")) + "/savg/dgl/etc/hev-grid/data/" + "hev-nearestObject.osg" ;
    }

    if (getenv("HEV_GRID_PROXIMITY_OBJECT"))
    {
	proximityObjectModelName = getenv("HEV_GRID_PROXIMITY_OBJECT") ;	
    }
    else
    {
	proximityObjectModelName = std::string(getenv("HEVROOT")) + "/savg/dgl/etc/hev-grid/data/" + "hev-proximityObject.osg" ;
    }

    if (getenv("HEV_GRID_WAND_OBJECT"))
    {
	wandObjectModelName = getenv("HEV_GRID_WAND_OBJECT") ;	
    }
    else
    {
	wandObjectModelName = std::string(getenv("HEVROOT")) + "/savg/dgl/etc/hev-grid/data/" + "hev-wandObject.osg" ;
    }

    if (getenv("HEV_GRID_CUBE_OBJECT"))
    {
	cubeObjectModelName = getenv("HEV_GRID_CUBE_OBJECT") ;	
    }
    else
    {
	cubeObjectModelName = std::string(getenv("HEVROOT")) + "/savg/dgl/etc/hev-grid/data/" + "hev-cubeObject.osg" ;
    }

    if (getenv("HEV_GRID_AXES_OBJECT"))
    {
	axesObjectModelName = getenv("HEV_GRID_AXES_OBJECT") ;	
    }
    else
    {
	axesObjectModelName = std::string(getenv("HEVROOT")) + "/savg/dgl/etc/hev-grid/data/" + "hev-axesObject.osg" ;
    }

    if (getenv("HEV_GRID_BACKGROUND_OBJECT"))
    {
	backgroundObjectModelName = getenv("HEV_GRID_BACKGROUND_OBJECT") ;	
    }
    else
    {
	backgroundObjectModelName = std::string(getenv("HEVROOT")) + "/savg/dgl/etc/hev-grid/data/" + "hev-backgroundObject.osg" ;
    }


    oldShowNearest = showNearest ;
    oldShowProximity = showProximity ;
    oldShowPicker = showPicker ;
    oldShowCube = showCube ;
    oldShowAxes = showAxes ;
    oldShowBackground = showBackground ;

    Fl_Double_Window* win = guiWindow() ;
    win->show(argc, argv);
    
    iris::Signal(signal_catcher) ;
    
    wandShm = new dtkSharedMem(sizeof(wand), wandShmName.c_str(), DTK_CONNECT) ;
    if (wandShm->isInvalid()) return 1 ;
    
    writeStingAndFlush("DCS " + gridDCSname) ;
    writeStingAndFlush("ADDCHILD " + gridDCSname + " scene") ;
    
    writeStingAndFlush("DCS " + wandDCSname) ;
    writeStingAndFlush("ADDCHILD " + wandDCSname + " scene") ;
    writeStingAndFlush("LOAD "+ wandObjectNodeName + " " +  wandObjectModelName) ;

    writeStingAndFlush("DCS " + nearestDCSname) ;
    writeStingAndFlush("ADDCHILD " + nearestDCSname + " " + gridDCSname) ;
    writeStingAndFlush("LOAD "+ nearestObjectNodeName + " " +  nearestObjectModelName) ;
    writeStingAndFlush("LOAD "+ proximityObjectNodeName + " " +  proximityObjectModelName) ;
    writeStingAndFlush("LOAD "+ backgroundObjectNodeName + " " +  backgroundObjectModelName) ;
    writeStingAndFlush("LOAD "+ cubeObjectNodeName + " " +  cubeObjectModelName) ;
    writeStingAndFlush("LOAD "+ axesObjectNodeName + " " +  axesObjectModelName) ;

    updateButtons() ;
    updateGridScale() ;

    Fl::add_timeout(.1, timeoutCallback) ;
    
    while(running && Fl::wait())
    {
    }

    return 0 ;
}
