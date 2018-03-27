#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <osg/Matrix>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <dtk.h>
#include <iris.h>

#define SCALEBOUNDARY (.75f)

// widgets
Fl_Double_Window *main_window ;
Fl_Light_Button *uniformScalingButton ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

// these need to match how fltk sets the radio buttons
bool rotation = false ;
bool translation = true ;
bool scale = false ;
bool uniformScaling = false ;

// set to true if --scale, --translate, --rotate --uniformscale specified
bool gui = true ;

// true if wand is in cube- set by toggleCube()
bool inside = false ;
bool oldInside = inside ;
enum INSIDE_SCALE { NONE, PX, MX, PY, MY, PZ, MZ} ;
INSIDE_SCALE insideScale = NONE ;
INSIDE_SCALE oldInsideScale = insideScale ;

// true if inside is true and button was pressed- set in timer_callback()
bool moving = false ;

// function templates
bool setup(int argc, char **argv) ;
void usage() ;
void updateTransformation(bool t, bool r, bool s) ;
void timer_callback(void*) ;
void toggleCube() ;
osg::Matrix worldToCube(osg::Matrix& w, osg::Matrix& t) ;

// set to name of shared memory if --shm option passed
std::string transformationShmName ;
dtkSharedMem *transformationShm = NULL ;
osg::Matrix transformation ;
// transformation when cube first starts to move
osg::Matrix initTransformation ;

// set to name of DCS node if --dcs option passed
std::string dcs = "hev-cubeTransformationWandDCS" ;

// set to name of world wand shared memory
std::string worldWandShmName = "idea/worldOffsetWand" ;
dtkSharedMem *worldWandShm  ;
osg::Matrix worldWand ;
osg::Matrix oldWorldWand ;
// world wand when cube first starts to move
osg::Matrix initWorldWand ;
osg::Matrix initWorldWandInv ;

// cube wand when cube first starts to move
osg::Matrix initCubeWand ;

// tells us if it's our turn to use the buttons
iris::ShmString* selector ;
std::string selectorShmName = "idea/selector" ;
// the name of our selector string
std::string selectorStr = "cube" ;

// read a button to move the cube
dtkSharedMem* buttonShm ;
std::string buttonShmName = "idea/buttons/left" ;
char button = false ;
char oldButton = false ;

// these are in xyz hpr sx sy sy
osg::Vec3d xyz(0., 0., 0.) ;
osg::Vec3d hpr(0., 0., 0.) ;
osg::Vec3d scaleXYZ(1., 1., 1.) ;

osg::Vec3d initXYZ(xyz) ;
osg::Vec3d initHPR(hpr) ;
osg::Vec3d initScaleXYZ(scaleXYZ) ;


////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-cubeTransformationWand [ --shm shm ] [ --worldWandShm wandShm ] [--buttonShm buttonShmName] [--selectorShm selectorShmName] [--selectorStr string] [ --noescape ] [ --xyz x y z ] [ --hpr h p r ] [ --scale xs ys zs ] [ --extent file ] [ --dcs nodename ] [ [ --translate ] [--rotate ] | [ --scale ] [ --uniformscale ] ]\n") ;
}

////////////////////////////////////////////////////////////////////////
bool setup(int argc, char **argv)
{

    int i = 1 ;
    //fprintf(stderr,"%d: ",argc) ;
    while (i<argc)
    {
	//fprintf(stderr,"%s ",argv[i]) ;
	if (iris::IsSubstring("--noescape",argv[i],5))
	{
	    noEscape = true ;
	    //~fprintf(stderr,("setting noescape\n") ;
	    i++ ;
	}

	else if (iris::IsSubstring("--shm",argv[i],5))
	{
	    i++ ;
	    if (i<argc)
	    {
		transformationShmName = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationWand: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--dcs",argv[i],5))
	{
	    i++ ;
	    if (i<argc)
	    {
		dcs = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationWand: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--worldwandshm",argv[i],5))
	{
	    i++ ;
	    if (i<argc)
	    {
		worldWandShmName = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationWand: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--buttonshm",argv[i],5))
	{
	    i++ ;
	    if (i<argc)
	    {
		buttonShmName = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationWand: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--selectorshm",argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		selectorShmName = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationWand: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--selectorstr",argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		selectorStr = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationWand: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--xyz",argv[i],5))
	{
	    i++ ;
	    if (i+2<argc)
	    {
		if ( (sscanf(argv[i],"%f",&(initXYZ.x())) != 1) || 
		     (sscanf(argv[i+1],"%f",&(initXYZ.y())) != 1) || 
		     (sscanf(argv[i+2],"%f",&(initXYZ.z())) != 1) )
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid x, y, or z parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i+=3 ;
	}
	
	else if (iris::IsSubstring("--hpr",argv[i],5))
	{
	    i++ ;
	    if (i+2<argc)
	    {
		if ( (sscanf(argv[i],"%f",&(initHPR[0])) != 1) || 
		     (sscanf(argv[i+1],"%f",&(initHPR[1])) != 1) || 
		     (sscanf(argv[i+2],"%f",&(initHPR[2])) != 1) )
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid h, p, or r parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i+=3 ;
	}
	
	else if (iris::IsSubstring("--scale",argv[i]))
	{
	    i++ ;
	    if (i+2<argc)
	    {
		if ( (sscanf(argv[i],"%f",&(initScaleXYZ.x())) != 1) || 
		     (sscanf(argv[i+1],"%f",&(initScaleXYZ.y())) != 1) || 
		     (sscanf(argv[i+2],"%f",&(initScaleXYZ.z())) != 1) )
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid xs, ys, or zs parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i+=3 ;
	}
	
	else if (iris::IsSubstring("--extent",argv[i],5))
	{
	    i++ ;
	    if (i<argc)
	    {
		osg::Group* group = new osg::Group ;
		osg::Node* node = osgDB::readNodeFile(argv[i]) ;
		if (!node)
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: can't load extent file\n") ;
		    return false ;
		}
		group->addChild(node) ;
		osg::BoundingBox bb = iris::GetBoundingBox(group);
		osg::Vec3d scaleObj((bb.xMax()-bb.xMin())/2.0,
				   (bb.yMax()-bb.yMin())/2.0,
				   (bb.zMax()-bb.zMin())/2.0) ;

		//printf("center = %f %f %f\n",bb.center().x(),bb.center().y(),bb.center().z()) ;
		//printf("scaleObj = %f %f %f\n",scaleObj.x(),scaleObj.y(),scaleObj.z()) ;
		initXYZ = bb.center() ;
		initScaleXYZ = scaleObj ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--translate",argv[i],5))
	{
	    i++ ;
	    translation = true ;
	    scale = false ;
	    uniformScaling = false ;
	    insideScale = NONE ;
	    gui = false ;
	}

	else if (iris::IsSubstring("--rotate",argv[i],5))
	{
	    i++ ;
	    if (gui) translation = false ;
	    rotation = true ;
	    scale = false ;
	    uniformScaling = false ;
	    insideScale = NONE ;
	    gui = false ;
	}

	else if (iris::IsSubstring("--scale",argv[i],5))
	{
	    i++ ;
	    translation = false ;
	    rotation = false ;
	    scale = true ;
	    uniformScaling = false ;
	    inside = false ;
	    gui = false ;
	}

	else if (iris::IsSubstring("--uniformscale",argv[i],5))
	{
	    i++ ;
	    translation = false ;
	    rotation = false ;
	    scale = true ;
	    uniformScaling = true ;
	    inside = false ;
	    gui = false ;
	}

	else
	{
	    fprintf(stderr, "hev-cubeTransformationWand: unknown argument\n") ;
	    return false ;
	}
	
    }

    
    xyz = initXYZ ;
    hpr = initHPR ;
    scaleXYZ = initScaleXYZ ;

    transformationShm = new dtkSharedMem(sizeof(transformation), transformationShmName.c_str()) ;
    if (transformationShm->isInvalid()) return false ;
	
    buttonShm = new dtkSharedMem(sizeof(char), buttonShmName.c_str());
    if (buttonShm->isInvalid()) return false ;

    selector = new iris::ShmString(selectorShmName) ;

    worldWandShm = new dtkSharedMem(sizeof(worldWand), worldWandShmName.c_str()) ;
    if (worldWandShm->isInvalid()) return false;

    transformation.preMultTranslate(xyz) ;
    transformation.preMultRotate(iris::EulerToQuat(hpr[0],hpr[1],hpr[2])) ;
    transformation.preMultScale(scaleXYZ) ;

    if (transformationShm) transformationShm->write(transformation.ptr()) ;

    printf("MATRIX %s %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n", dcs.c_str(),
	   transformation(0,0), transformation(0,1), transformation(0,2), transformation(0,3),
	   transformation(1,0), transformation(1,1), transformation(1,2), transformation(1,3),
	   transformation(2,0), transformation(2,1), transformation(2,2), transformation(2,3),
	   transformation(3,0), transformation(3,1), transformation(3,2), transformation(3,3)) ;

    printf("LOAD hev-cubeTransformationWand-highlightCube highlightCube.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightCube OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightCubeNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightCube hev-cubeTransformationWand-highlightCubeNoClip\n") ;

    printf("LOAD hev-cubeTransformationWand-highlightSquare+X highlightSquare+X.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightSquare+X OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightSquare+XNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare+X hev-cubeTransformationWand-highlightSquare+XNoClip\n") ;

    printf("LOAD hev-cubeTransformationWand-highlightSquare-X highlightSquare-X.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightSquare-X OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightSquare-XNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare-X hev-cubeTransformationWand-highlightSquare-XNoClip\n") ;

    printf("LOAD hev-cubeTransformationWand-highlightSquare+Y highlightSquare+Y.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightSquare+Y OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightSquare+YNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare+Y hev-cubeTransformationWand-highlightSquare+YNoClip\n") ;

    printf("LOAD hev-cubeTransformationWand-highlightSquare-Y highlightSquare-Y.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightSquare-Y OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightSquare-YNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare-Y hev-cubeTransformationWand-highlightSquare-YNoClip\n") ;

    printf("LOAD hev-cubeTransformationWand-highlightSquare+Z highlightSquare+Z.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightSquare+Z OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightSquare+ZNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare+Z hev-cubeTransformationWand-highlightSquare+ZNoClip\n") ;

    printf("LOAD hev-cubeTransformationWand-highlightSquare-Z highlightSquare-Z.osgb\n") ;
    printf("NODEMASK hev-cubeTransformationWand-highlightSquare-Z OFF\n") ;

    printf("NOCLIP hev-cubeTransformationWand-highlightSquare-ZNoClip\n") ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare-Z hev-cubeTransformationWand-highlightSquare-ZNoClip\n") ;

    printf("ADDCHILD hev-cubeTransformationWand-highlightCubeNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare+XNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare-XNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare+YNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare-YNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare+ZNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD hev-cubeTransformationWand-highlightSquare-ZNoClip  %s\n", dcs.c_str()) ;
    printf("ADDCHILD %s  world\n", dcs.c_str()) ;

    fflush(stdout) ;

    Fl::add_timeout(.01, timer_callback) ;

    return true ;
}
 
////////////////////////////////////////////////////////////////////////
void timer_callback(void*)
{

    // are we active?
    //~fprintf(stderr,"selector string = %s, selectorStr = %s, selectorShm = %s\n",selector->getString().c_str(),selectorStr.c_str(),  selector->getShmName().c_str()) ;
    if (selector->getString() == selectorStr)
    {
	// read the wand in world coordinates
	worldWandShm->read(worldWand.ptr()) ;

	// turn cube on and off depending on where wand and cube are
	// sets global variables "inside*"
	if (!moving) toggleCube() ;
	
	// read the "move the cube" button 
	buttonShm->read(&button) ;

	//fprintf(stderr,"inside = %d, insideScale = %d\n",inside, insideScale) ;
	if ((inside || insideScale!=NONE) && button != 0 && oldButton == 0) //press
	{
	    initWorldWand = worldWand ;
	    initWorldWandInv = osg::Matrix::inverse(initWorldWand) ;
	    initTransformation = transformation ;
	    initCubeWand = worldToCube(initWorldWand, initTransformation) ;
	    moving = true ;
	}
	else if (moving && button != 0 && oldButton != 0) //still down
	{
	    // wand moved
	    if (oldWorldWand != worldWand)
	    {
		oldWorldWand = worldWand ;
		updateTransformation(translation, rotation, scale) ;
	    }
	    
	}
	else if (button == 0 && oldButton != 0) //release
	{
	    moving = false ;
	}
	
	oldButton = button ;
    }

    Fl::repeat_timeout(.01, timer_callback) ;

}


////////////////////////////////////////////////////////////////////////
void updateTransformation(bool t, bool r, bool s)
{

    // get the transformation that's the diffrence between the currrent wand position and the initial wand position
    osg::Matrix delta = worldWand * initWorldWandInv ;

    osg::Vec3d deltaT ;
    osg::Quat deltaQ  ;
    osg::Vec3d deltaS ; 
    iris::Decompose(delta, &deltaT, &deltaQ, &deltaS) ;
 
    osg::Vec3d transformationT ;
    osg::Quat transformationQ  ;
    osg::Vec3d transformationS ; 
    iris::Decompose(transformation, &transformationT, &transformationQ, &transformationS) ;

    osg::Vec3d initTransformationT ;
    osg::Quat initTransformationQ  ;
    osg::Vec3d initTransformationS ; 
    iris::Decompose(initTransformation, &initTransformationT, &initTransformationQ, &initTransformationS) ;

    osg::Matrix result = initTransformation ;

    if (r && inside) // do rotation
    {
	result.postMultRotate(deltaQ) ;
    }

    if (t && inside) // then translation
    {
	result.postMultTranslate(deltaT) ;
    }


    //fprintf(stderr,"insideScale = %d\n",insideScale) ;

    if (s && insideScale != NONE) //scale along a specific wall
    {

	// the scale is calculated from initial values every frame

	// get wand in coordinate system of cube
	osg::Matrix cubeWand = worldToCube(worldWand, initTransformation) ;

	if (initCubeWand != cubeWand) 
	{

	    int i = -1 ;
	    if (insideScale == MX || insideScale == PX)
	    {
		i = 0 ;
	    }
	    else if (insideScale == MY || insideScale == PY)
	    {
		i = 1 ;
	    }
	    else if (insideScale == MZ || insideScale == PZ)
	    {
		i = 2 ;
	    }
	    
	    if (i==-1)
	    {
		fprintf(stderr, "hev-cubeTransformationWandFunctions::updateTransformation: logic error- oops!!\n") ;
		exit(1) ;
	    }


	    fprintf(stderr,"i = %d\n",i) ;

	    // change in scale & translation is related to change in wand translation since button was pressed
	    osg::Vec3d cubeWandT = cubeWand.getTrans() ;
	    osg::Vec3d initCubeWandT = initCubeWand.getTrans() ;

	    // you might want to change these until they look right in the RAVE
	    double scaleFactor = (cubeWandT[i]/initCubeWandT[i]) ;
	    double transFactor = (scaleFactor - 1.0)*.5 ;
	    // change translation direction if using a negative axis
	    if (insideScale == MX || insideScale == MY || insideScale == MZ) transFactor = -transFactor ;
	    fprintf(stderr,"scaleFactor = %f,transFactor = %f\n", scaleFactor, transFactor) ;

	    osg::Vec3d scaledTrans(transformationT) ;
	    osg::Vec3d scaledScale(transformationS) ;

	    fprintf(stderr,"before scaling, scaledTrans[%d] = %f, scaledScale[%d] = %f\n",i,scaledTrans[i],i,scaledScale[i]) ;
	    fprintf(stderr,"cubeWandT[%d] = %f\n",i,cubeWandT[i]) ;
	    fprintf(stderr,"initCubeWandT[%d] = %f\n",i,initCubeWandT[i]) ;
	    if (!uniformScaling) // just scale in one direction
	    {
		scaledTrans[i] = initTransformationT[i] + initTransformationS[i]*transFactor ;
		scaledScale[i] = initTransformationS[i]*scaleFactor ;
	    }
	    else
	    {
		// need to get length of vector?
		// uniform scaling doesn't look right if translating- just scale around center
		//scaledTrans = initTransformationT + initTransformationS*transFactor ;
		scaledScale = initTransformationS*scaleFactor ;
	    }
	    fprintf(stderr,"after scaling, scaledTrans[%d] = %f, scaledScale[%d] = %f\n\n",i,scaledTrans[i],i,scaledScale[i]) ;

	    result.postMultTranslate(scaledTrans) ;
	    result.preMultScale(scaledScale) ;
	}

    }
    
    ////////////////////////////////////////////////////////////////////////    

    transformation = result ;

    if (transformationShm) transformationShm->write(transformation.ptr()) ;

    printf("MATRIX %s %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n", dcs.c_str(),
	   transformation(0,0), transformation(0,1), transformation(0,2), transformation(0,3),
	   transformation(1,0), transformation(1,1), transformation(1,2), transformation(1,3),
	   transformation(2,0), transformation(2,1), transformation(2,2), transformation(2,3),
	   transformation(3,0), transformation(3,1), transformation(3,2), transformation(3,3)) ;

    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void toggleCube()
{

    // get wand position in cube coordinates
    osg::Vec3d cubeWandTrans = worldToCube(worldWand, transformation).getTrans() ;

    //fprintf(stderr,"cubeWandTrans at %f %f %f\n", cubeWandTrans.x(),  cubeWandTrans.y(),  cubeWandTrans.z()) ;

    // is the wandinside the box?
    if (cubeWandTrans.x() >= -1.f && cubeWandTrans.x() <= 1.f &&
	cubeWandTrans.y() >= -1.f && cubeWandTrans.y() <= 1.f &&
	cubeWandTrans.z() >= -1.f && cubeWandTrans.z() <= 1.f)
    {
	if (scale) // need to make extra checks
	{
	    if (cubeWandTrans.x() >= SCALEBOUNDARY &&
		cubeWandTrans.y() >= -SCALEBOUNDARY && cubeWandTrans.y() <= SCALEBOUNDARY &&
		cubeWandTrans.z() >= -SCALEBOUNDARY && cubeWandTrans.z() <= SCALEBOUNDARY)
	    {
		insideScale = PX ;
	    }
	    else if (cubeWandTrans.x() <= -SCALEBOUNDARY &&
		     cubeWandTrans.y() >= -SCALEBOUNDARY && cubeWandTrans.y() <= SCALEBOUNDARY &&
		     cubeWandTrans.z() >= -SCALEBOUNDARY && cubeWandTrans.z() <= SCALEBOUNDARY)
	    {
		insideScale = MX ;
	    }
	    else if (cubeWandTrans.y() >= SCALEBOUNDARY &&
		     cubeWandTrans.x() >= -SCALEBOUNDARY && cubeWandTrans.x() <= SCALEBOUNDARY &&
		     cubeWandTrans.z() >= -SCALEBOUNDARY && cubeWandTrans.z() <= SCALEBOUNDARY)
	    {
		insideScale = PY ;
	    }
	    else if (cubeWandTrans.y() <= -SCALEBOUNDARY &&
		     cubeWandTrans.x() >= -SCALEBOUNDARY && cubeWandTrans.x() <= SCALEBOUNDARY &&
		     cubeWandTrans.z() >= -SCALEBOUNDARY && cubeWandTrans.z() <= SCALEBOUNDARY)
	    {
		insideScale = MY ;
	    }
	    else if (cubeWandTrans.z() >= SCALEBOUNDARY &&
		     cubeWandTrans.x() >= -SCALEBOUNDARY && cubeWandTrans.x() <= SCALEBOUNDARY &&
		     cubeWandTrans.y() >= -SCALEBOUNDARY && cubeWandTrans.y() <= SCALEBOUNDARY)
	    {
		insideScale = PZ ;
	    }
	    else if (cubeWandTrans.z() <= -SCALEBOUNDARY &&
		     cubeWandTrans.x() >= -SCALEBOUNDARY && cubeWandTrans.x() <= SCALEBOUNDARY &&
		     cubeWandTrans.y() >= -SCALEBOUNDARY && cubeWandTrans.y() <= SCALEBOUNDARY)
	    {
		insideScale = MZ ;
	    }
	    else
	    {
		insideScale = NONE ;	
	    }
	}
	else // that's good enough if not scaling
	{
	    insideScale = NONE ;
	    inside = true ;
	}
    }
    else
    {
	insideScale = NONE ;	
	inside = false ;
    }

    if (insideScale != oldInsideScale)
    {
	printf("NODEMASK hev-cubeTransformationWand-highlightSquare+X OFF\n") ;
	printf("NODEMASK hev-cubeTransformationWand-highlightSquare-X OFF\n") ;
	printf("NODEMASK hev-cubeTransformationWand-highlightSquare+Y OFF\n") ;
	printf("NODEMASK hev-cubeTransformationWand-highlightSquare-Y OFF\n") ;
	printf("NODEMASK hev-cubeTransformationWand-highlightSquare+Z OFF\n") ;
	printf("NODEMASK hev-cubeTransformationWand-highlightSquare-Z OFF\n") ;
	if (insideScale == PX) 
	{
	    printf("NODEMASK hev-cubeTransformationWand-highlightSquare+X ON\n") ;
	}
	else if (insideScale == MX) 
	{
	    printf("NODEMASK hev-cubeTransformationWand-highlightSquare-X ON\n") ;
	}
	else if (insideScale == PY) 
	{
	    printf("NODEMASK hev-cubeTransformationWand-highlightSquare+Y ON\n") ;
	}
	else if (insideScale == MY) 
	{
	    printf("NODEMASK hev-cubeTransformationWand-highlightSquare-Y ON\n") ;
	}
	else if (insideScale == PZ) 
	{
	    printf("NODEMASK hev-cubeTransformationWand-highlightSquare+Z ON\n") ;
	}
	else if (insideScale == MZ) 
	{
	    printf("NODEMASK hev-cubeTransformationWand-highlightSquare-Z ON\n") ;
	}
	oldInsideScale = insideScale ;
    }

   if (inside != oldInside)
   {
       if (inside) printf("NODEMASK hev-cubeTransformationWand-highlightCube ON\n") ;
       else printf("NODEMASK hev-cubeTransformationWand-highlightCube OFF\n") ;
       oldInside  = inside ;
   }


    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
// transform the wand from world cordinatesto cube coordinates (-1 to 1 each axis)
osg::Matrix worldToCube(osg::Matrix& w, osg::Matrix& t)
{
    return w * osg::Matrix::inverse(t) ;
}
