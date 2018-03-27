#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Box.H>

#include <dtk.h>

#include <osg/Matrix>
#include <osg/Vec3>
#include <osgDB/WriteFile>

#include <iris.h>

#define TIMER_TIMEOUT (1.f/2000.f)

// widgets
Fl_Double_Window *main_window, *objectDescription_window ;
Fl_Round_Button *cubeButton, *sphereButton, *tetrahedronButton, *pyramidButton ;
Fl_Check_Button *insideButton, *outsideButton ;
Fl_Roller *shellRoller ;
Fl_Value_Output *shellOutput ;
Fl_Multiline_Output *objectDescription;

// create the object description window
void make_objectDescription_window() ;
// and update the description
void update_objectDescription_window() ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

// how many points do we have?
unsigned int numPoints = 0 ;

// how many are selected?
unsigned int numPointsSelected = 0 ;

bool outside = true ;
bool oldOutside = outside ;

bool wireframe = true ;
bool oldWireframe = !wireframe ;

bool translucent = false ;
bool oldTranslucent = !translucent ;

bool shell = false ;
bool oldShell = !shell ;

// tell updateShapes to update the MATRIX node if the transformation changes
bool updateMATRIX = true ;

// indicate that you need to write data because the points have changed
bool writeSelectedDataNeeded = false ;

dtkSharedMem *transformationShm ;
std::string transformationShmName ;
osg::Matrix transformation ;
osg::Matrix oldTransformation ;
osg::Matrix transformationInv ;

std::string pointsFileName ;
osg::Vec3d* points = NULL ;
bool bpoints ;

dtkSharedMem *eligibleShm ;
std::string eligibleShmName ;
unsigned char* eligible ;
unsigned char* oldEligible ;

dtkSharedMem *selectedShm ;
std::string selectedShmName ;
unsigned char* selected ;

bool inside = true ;
bool oldInside = inside ;

enum OBJECT { NONE, CUBE, SPHERE, TETRAHEDRON, PYRAMID } ;
OBJECT object = CUBE ;
OBJECT oldObject = NONE ;

double thickness = .5f ;
double oldThickness = thickness ;

// read in the points that define the tetrahedron
double tetrahedronPoints[4][3] = {
#include "tetrahedron.array"
} ;

// convert to 4 vertices
osg::Vec3d tetrahedronVertices[4] = { 
    osg::Vec3(tetrahedronPoints[0][0], tetrahedronPoints[0][1], tetrahedronPoints[0][2]), 
    osg::Vec3(tetrahedronPoints[1][0], tetrahedronPoints[1][1], tetrahedronPoints[1][2]), 
    osg::Vec3(tetrahedronPoints[2][0], tetrahedronPoints[2][1], tetrahedronPoints[2][2]), 
    osg::Vec3(tetrahedronPoints[3][0], tetrahedronPoints[3][1], tetrahedronPoints[3][2]) } ;

// make 4 planes, one for each face of the tetrahedron
osg::Plane tetrahedronFaces[4] = {
    osg::Plane(tetrahedronVertices[0], tetrahedronVertices[1], tetrahedronVertices[2]),
    osg::Plane(tetrahedronVertices[0], tetrahedronVertices[3], tetrahedronVertices[1]),
    osg::Plane(tetrahedronVertices[0], tetrahedronVertices[2], tetrahedronVertices[3]),
    osg::Plane(tetrahedronVertices[1], tetrahedronVertices[3], tetrahedronVertices[2]) } ;

// the pyramid vertices are a bit more predictable
double sqrt3 = sqrt(3) ;

osg::Vec3d pyramidVertices[5] = { 
    osg::Vec3(0, 0, sqrt3), 
    osg::Vec3(1, 1, -1), osg::Vec3(-1, 1, -1), osg::Vec3(-1,-1, -1), osg::Vec3(1,-1, -1) } ;

// make the four pyramid faces
osg::Plane pyramidFaces[4] = {
    osg::Plane(pyramidVertices[0], pyramidVertices[1], pyramidVertices[2]), 
    osg::Plane(pyramidVertices[0], pyramidVertices[2], pyramidVertices[3]), 
    osg::Plane(pyramidVertices[0], pyramidVertices[3], pyramidVertices[4]), 
    osg::Plane(pyramidVertices[0], pyramidVertices[4], pyramidVertices[1]) } ;

// function templates
bool setup(int argc, char **argv) ;
void usage() ;
void timer_callback(void*) ;
void updateSelection() ;
void writeSelectedData() ;
bool readEligibleData() ;
void loadShapes() ;
void updateShapes() ;
void updatePoints() ;
bool isVisible(osg::Vec3d) ;
bool testPoint(osg::Vec3d v) ;
void setShell(bool) ;
void shellThickness(double) ;
void cleanup() ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-shapeSelection --transformation shmName --points fileName --eligible shmName --selected shmname --noescape\n") ;
}

////////////////////////////////////////////////////////////////////////
bool setup(int argc, char **argv)
{
    
    int i = 1 ;
    while (i<argc)
    {
	if (iris::IsSubstring("--noescape",argv[i],3))
	{
	    noEscape = true ;
	    //~fprintf(stderr,("setting noescape\n") ;
	    i++ ;
	}

	else if (iris::IsSubstring("--transformation",argv[i],3))
	{
	    i++ ;
	    if (i<argc)
	    {
		transformationShmName = argv[i] ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--points",argv[i],3))
	{
	    i++ ;
	    if (i<argc)
	    {
		pointsFileName = argv[i] ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: not enough arguments\n") ;
		return false ;
	    }
	    bpoints = false ;
	    i++ ;
	}

	else if (iris::IsSubstring("--bpoints",argv[i],3))
	{
	    i++ ;
	    if (i<argc)
	    {
		pointsFileName = argv[i] ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: not enough arguments\n") ;
		return false ;
	    }
	    bpoints = true ;
	    i++ ;
	}

	else if (iris::IsSubstring("--eligible",argv[i],3))
	{
	    i++ ;
	    if (i<argc)
	    {
		eligibleShmName = argv[i] ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--selected",argv[i],3))
	{
	    i++ ;
	    if (i<argc)
	    {
		selectedShmName = argv[i] ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}
	else
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: unknown option\n") ;
	    return false ;
	}
    }
    
    if (transformationShmName == "")
    {
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: --transformation not supplied\n") ;
	    return false ;
    }

    if (pointsFileName == "")
    {
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: --points or --bpoints not supplied\n") ;
	    return false ;
    }

    if (eligibleShmName == "")
    {
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: --eligible not supplied\n") ;
	    return false ;
    }

    if (selectedShmName == "")
    {
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: --selected not supplied\n") ;
	    return false ;
    }

    // shape transformation
    transformationShm = new dtkSharedMem(sizeof(transformation), transformationShmName.c_str()) ;
    if (transformationShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't open transformation shared memory %s\n",transformationShmName.c_str()) ;
	return false ;
    }
    transformationShm->read(transformation.ptr()) ;
#if 0
    {
	osg::Vec3d t = transformation.getTrans() ;
	dtkMsg.add(DTKMSG_WARNING, "hev-shapeSelection: initial translation %f %f %f\n",t.x(), t.y(), t.z()) ;
    }
#endif
    // points file
    if (bpoints)
    {
	FILE* pointsFile = fopen(pointsFileName.c_str(),"r");
	if (!pointsFile)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't open binary points file %s\n",pointsFileName.c_str()) ;
	    return false ;
	}

	if (fread(&numPoints,sizeof(unsigned int),1,pointsFile) != 1)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't read number of points from binary file %s\n",pointsFileName.c_str()) ;
	    return false ;
	}
	unsigned int poinstSize = numPoints*sizeof(osg::Vec3d) ;
	points = (osg::Vec3d*) malloc(poinstSize) ;
	if (fread(points,poinstSize,1,pointsFile) != 1)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't read %d points from binary file %s\n",numPoints, pointsFileName.c_str()) ;
	    return false ;
	}
#if 0
	fprintf(stderr,"%d points read\n",numPoints) ;	
	for (int i=0; i<numPoints; i++) 
	{
	    fprintf(stderr,"  %d: %f %f %f\n",i,points[i].x(),points[i].y(),points[i].z()) ;
	}
#endif
    }
    else
    {
	std::ifstream pointsFile ;
	pointsFile.open(pointsFileName.c_str()) ;
	if (pointsFile.fail())
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't open formatted points file %s\n",pointsFileName.c_str()) ;
	    return false ;
	}
	std::string line ;
	std::vector<std::string> words ;
	while (getline(pointsFile,line))
	{
	    words = iris::ParseString(line) ;
	    if (words.size() != 3)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: invalid line \"%s\"\n",line.c_str()) ;
		return 1 ;
	    }
	    osg::Vec3d point ;
	    if (!iris::StringToDouble(words[0], &(point.x())) || !iris::StringToDouble(words[1], &(point.y())) || !iris::StringToDouble(words[2], &(point.z())))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: invalid number in line \"%s\"\n",line.c_str()) ;
		return 1 ;
	    }
	    unsigned int pointsSize = sizeof(osg::Vec3d)*(numPoints+1) ;
	    points = (osg::Vec3d*) realloc(points, pointsSize) ;
	    *(points+numPoints) = point ;
	    numPoints++ ;
	}
    }

    dtkMsg.add(DTKMSG_INFO, "hev-shapeSelection: %d points read\n",numPoints) ;
    eligibleShm = new dtkSharedMem(numPoints, eligibleShmName.c_str()) ;
    if (eligibleShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't open eligible shared memory %s\n",eligibleShmName.c_str()) ;
	return false ;
    }
    eligibleShm->writeCount(1) ;
    eligible = (unsigned char*) malloc(numPoints) ;
    oldEligible = (unsigned char*) malloc(numPoints) ;
    memset(eligible, 0, sizeof(numPoints)) ;
    memset(oldEligible, 0, sizeof(numPoints)) ;
    eligibleShm->read(eligible) ;
#if 0
    {
	int c = 0 ;
	for (int i = 0; i<numPoints; i++) if (eligible[i]) c++ ;
	dtkMsg.add(DTKMSG_WARNING,"hev-shapeSelection: %d eligible points at initialization\n", c) ;
    }
#endif
    // and also selected
    selectedShm = new dtkSharedMem(numPoints, selectedShmName.c_str()) ;
    selected = (unsigned char*) malloc(numPoints) ;
    memset(selected, 0, sizeof(selected)) ;
    selectedShm->write(selected) ;
    if (selectedShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-shapeSelection: can't open selected shared memory %s\n",selectedShmName.c_str()) ;
	return false ;
    }

    loadShapes() ;

    Fl::add_timeout(TIMER_TIMEOUT, timer_callback) ;

    return true ;
}
 
////////////////////////////////////////////////////////////////////////
void timer_callback(void*)
{

    // don't turn on timer until i/o done!

    // read in eligible points
    while (readEligibleData())
    {
	writeSelectedDataNeeded = true ;
    }

    updateSelection() ;

    Fl::repeat_timeout(TIMER_TIMEOUT, timer_callback) ;

}

////////////////////////////////////////////////////////////////////////
void updateSelection()
{

    //fprintf(stderr,"hev-shapeSelection: entering updateSelection()\n") ;
    updateShapes() ;

    if (inside != oldInside || outside != oldOutside || shell != oldShell || thickness != oldThickness || oldObject != object)
    {
	//fprintf(stderr,"updating points because object changed\n") ;
	writeSelectedDataNeeded = true ;
	oldInside = inside ;
	oldOutside = outside ;
	oldShell = shell ;
	oldThickness = thickness ;
	oldObject = object ;
    }

    transformationShm->read(transformation.ptr()) ;

    if (oldTransformation != transformation)
    {
#if 0
	{
	    osg::Vec3d t = transformation.getTrans() ;
	    dtkMsg.add(DTKMSG_WARNING, "hev-shapeSelection: new translation %f %f %f\n",t.x(), t.y(), t.z()) ;
	}
#endif
	//fprintf(stderr,"updating points because object moved\n") ;
	writeSelectedDataNeeded = true ;

	transformationInv.invert(transformation) ;

	oldTransformation = transformation ;

	updateMATRIX = true ;

	update_objectDescription_window() ;
    }
    if (writeSelectedDataNeeded)
    {
	updatePoints() ;
    }
    //fprintf(stderr,"hev-shapeSelection: leaving updateSelection()\n") ;
}

////////////////////////////////////////////////////////////////////////
void loadShapes()
{

    // load shapes into scenegraph
    // a MATRIX at the top
    printf("MATRIX hev-shapeSelection-object-MATRIX  %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
	   transformation(0,0), transformation(0,1), transformation(0,2), transformation(0,3),
	   transformation(1,0), transformation(1,1), transformation(1,2), transformation(1,3),
	   transformation(2,0), transformation(2,1), transformation(2,2), transformation(2,3),
	   transformation(3,0), transformation(3,1), transformation(3,2), transformation(3,3)) ;
    printf("ADDCHILD hev-shapeSelection-object-MATRIX world\n") ;

    // under that, one node for wireframe and one node for translucent
    printf("GROUP hev-shapeSelection-wireframe\n") ;
    printf("ADDCHILD hev-shapeSelection-wireframe hev-shapeSelection-object-MATRIX\n") ;

    printf("GROUP hev-shapeSelection-translucent\n") ;
    printf("ADDCHILD hev-shapeSelection-translucent hev-shapeSelection-object-MATRIX\n") ;

    // and shell nodes under each of those
    osg::Matrix t = osg::Matrix::scale(osg::Vec3(thickness, thickness, thickness)) ;
    printf("MATRIX hev-shapeSelection-shell-wireframe  %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
	   t(0,0), t(0,1), t(0,2), t(0,3),
	   t(1,0), t(1,1), t(1,2), t(1,3),
	   t(2,0), t(2,1), t(2,2), t(2,3),
	   t(3,0), t(3,1), t(3,2), t(3,3)) ;
    printf("ADDCHILD hev-shapeSelection-shell-wireframe hev-shapeSelection-wireframe\n") ;

    printf("MATRIX hev-shapeSelection-shell-translucent %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
	   t(0,0), t(0,1), t(0,2), t(0,3),
	   t(1,0), t(1,1), t(1,2), t(1,3),
	   t(2,0), t(2,1), t(2,2), t(2,3),
	   t(3,0), t(3,1), t(3,2), t(3,3)) ;
    printf("ADDCHILD hev-shapeSelection-shell-translucent hev-shapeSelection-translucent\n") ;

    // hack for testing in home directory
    //std::string dir = "./" ;
    std::string dir = std::string(getenv("HEVROOT")) +  std::string("/idea/etc/hev-shapeSelection/data/") ;

    // load the shapes
    printf("LOAD hev-shapeSelection-cube %scube.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-cube hev-shapeSelection-wireframe\n") ;
    printf("ADDCHILD hev-shapeSelection-cube hev-shapeSelection-shell-wireframe\n") ;
    printf("LOAD hev-shapeSelection-cubeT %scubeT.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-cubeT hev-shapeSelection-translucent\n") ;
    printf("ADDCHILD hev-shapeSelection-cubeT hev-shapeSelection-shell-translucent\n") ;

    printf("LOAD hev-shapeSelection-sphere %ssphere.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-sphere hev-shapeSelection-wireframe\n") ;
    printf("ADDCHILD hev-shapeSelection-sphere hev-shapeSelection-shell-wireframe\n") ;
    printf("LOAD hev-shapeSelection-sphereT %ssphereT.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-sphereT hev-shapeSelection-translucent\n") ;
    printf("ADDCHILD hev-shapeSelection-sphereT hev-shapeSelection-shell-translucent\n") ;

    printf("LOAD hev-shapeSelection-pyramid %spyramid.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-pyramid hev-shapeSelection-wireframe\n") ;
    printf("ADDCHILD hev-shapeSelection-pyramid hev-shapeSelection-shell-wireframe\n") ;
    printf("LOAD hev-shapeSelection-pyramidT %spyramidT.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-pyramidT hev-shapeSelection-translucent\n") ;
    printf("ADDCHILD hev-shapeSelection-pyramidT hev-shapeSelection-shell-translucent\n") ;

    printf("LOAD hev-shapeSelection-tetrahedron %stetrahedron.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-tetrahedron hev-shapeSelection-wireframe\n") ;
    printf("ADDCHILD hev-shapeSelection-tetrahedron hev-shapeSelection-shell-wireframe\n") ;
    printf("LOAD hev-shapeSelection-tetrahedronT %stetrahedronT.osgb\n",dir.c_str()) ;
    printf("ADDCHILD hev-shapeSelection-tetrahedronT hev-shapeSelection-translucent\n") ;
    printf("ADDCHILD hev-shapeSelection-tetrahedronT hev-shapeSelection-shell-translucent\n") ;

    updateShapes() ;

    fflush(stdout) ;

}

////////////////////////////////////////////////////////////////////////
void updatePoints()
{

    //fprintf(stderr, "hev-shapeSelection: entering updatePoints()\n") ;

    numPointsSelected = 0 ;
    for (unsigned int i=0; i<numPoints; i++)
    {
	if (eligible[i] && isVisible(points[i]))
	{
	    selected[i] = true ;
	    numPointsSelected++ ;
	}
	else
	{
	    selected[i] = false ;
	}
    }
    update_objectDescription_window() ;
    writeSelectedData() ;

    //fprintf(stderr, "exiting updatepPoints()\n") ;
}
	
////////////////////////////////////////////////////////////////////////
void updateShapes()
{

    if (wireframe != oldWireframe)
    {
	if (!wireframe)
	{
	    //fprintf(stderr,"turning off wireframe\n") ;
	    printf("NODEMASK hev-shapeSelection-wireframe OFF\n") ;
	}
	else
	{
	    //fprintf(stderr,"turning on wireframe\n") ;
	    printf("NODEMASK hev-shapeSelection-wireframe ON\n") ;
	}
    }

    if (translucent != oldTranslucent)
    {
	if (!translucent)
	{
	    //fprintf(stderr,"turning off translucent\n") ;
	    printf("NODEMASK hev-shapeSelection-translucent OFF\n") ;
	}
	else
	{
	    //fprintf(stderr,"turning on translucent\n") ;
	    printf("NODEMASK hev-shapeSelection-translucent ON\n") ;
	}
    }

    if (shell != oldShell)
    {
	if (!shell)
	{
	    //fprintf(stderr,"turning off shell\n") ;
	    printf("NODEMASK hev-shapeSelection-shell-wireframe OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-shell-translucent OFF\n") ;
	}
	else
	{
	    //fprintf(stderr,"turning on shell\n") ;
	    printf("NODEMASK hev-shapeSelection-shell-wireframe ON\n") ;
	    printf("NODEMASK hev-shapeSelection-shell-translucent ON\n") ;
	}
    }

    if (object != oldObject)
    {
	if (object == CUBE)
	{
	    //fprintf(stderr,"turning on cube\n") ;
	    printf("NODEMASK hev-shapeSelection-cube ON\n") ;
	    printf("NODEMASK hev-shapeSelection-sphere OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramid OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedron OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-cubeT ON\n") ;
	    printf("NODEMASK hev-shapeSelection-sphereT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramidT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedronT OFF\n") ;
	}
	else if (object == SPHERE)
	{
	    //fprintf(stderr,"turning on sphere\n") ;
	    printf("NODEMASK hev-shapeSelection-cube OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-sphere ON\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramid OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedron OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-cubeT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-sphereT ON\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramidT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedronT OFF\n") ;
	}
	else if (object == TETRAHEDRON)
	{
	    //fprintf(stderr,"turning on tetrahedron\n") ;
	    printf("NODEMASK hev-shapeSelection-cube OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-sphere OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramid OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedron ON\n") ;
	    printf("NODEMASK hev-shapeSelection-cubeT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-sphereT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramidT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedronT ON\n") ;
	}
	else if (object == PYRAMID)
	{
	    //fprintf(stderr,"turning on pyramid\n") ;
	    printf("NODEMASK hev-shapeSelection-cube OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-sphere OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramid ON\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedron OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-cubeT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-sphereT OFF\n") ;
	    printf("NODEMASK hev-shapeSelection-pyramidT ON\n") ;
	    printf("NODEMASK hev-shapeSelection-tetrahedronT OFF\n") ;
	}

    }

    //oldObject = object ;  // gets done in update()
    oldWireframe = wireframe ;
    //oldShell = shell ;  // gets done in update()
    oldWireframe = wireframe ;
    oldTranslucent = translucent ;


    if (updateMATRIX)
    {
	printf("MATRIX hev-shapeSelection-object-MATRIX %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
	   transformation(0,0), transformation(0,1), transformation(0,2), transformation(0,3),
	   transformation(1,0), transformation(1,1), transformation(1,2), transformation(1,3),
	   transformation(2,0), transformation(2,1), transformation(2,2), transformation(2,3),
	   transformation(3,0), transformation(3,1), transformation(3,2), transformation(3,3)) ;
	//fprintf(stderr,"updateShapes() updating MATRIX\n") ;
	updateMATRIX = false ;
    }

    fflush(stdout) ;

}
	
////////////////////////////////////////////////////////////////////////
void setShell(bool b) 
{
    if (b==shell) return ;
    shell = b ;

    if (b)
    {
	shellRoller->activate() ;
	shellOutput->activate() ;
	insideButton->deactivate() ;
	outsideButton->deactivate() ;
    }
    else
    {
	shellRoller->deactivate() ;
	shellOutput->deactivate() ;
	insideButton->activate() ;
	outsideButton->activate() ;
    }

}

////////////////////////////////////////////////////////////////////////
void shellThickness(double f)
{
    //thickness = (50.f - f)/100.f + .5f;
    thickness = f/100.f ;
    //fprintf(stderr, "f = %f, thickness = %f\n",f,thickness) ;
    osg::Matrix t = osg::Matrix::scale(osg::Vec3(thickness, thickness, thickness)) ;
    printf("MATRIX hev-shapeSelection-shell-wireframe  %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
	   t(0,0), t(0,1), t(0,2), t(0,3),
	   t(1,0), t(1,1), t(1,2), t(1,3),
	   t(2,0), t(2,1), t(2,2), t(2,3),
	   t(3,0), t(3,1), t(3,2), t(3,3)) ;
    printf("MATRIX hev-shapeSelection-shell-translucent %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
	   t(0,0), t(0,1), t(0,2), t(0,3),
	   t(1,0), t(1,1), t(1,2), t(1,3),
	   t(2,0), t(2,1), t(2,2), t(2,3),
	   t(3,0), t(3,1), t(3,2), t(3,3)) ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
bool isVisible(osg::Vec3d xyz)
{

    if (shell)
    {
	bool saveInside = inside ;
	bool saveOutside = outside ;

	// is it inside the shell?
	inside = true ; outside = false ;
	osg::Vec3d v = xyz * transformationInv ;
	bool insideShape = testPoint(v) ;
	if (!insideShape) return false ;

	// is it outside the shell?
	inside = false ; outside = true ;
	//fprintf(stderr,"\nshape v = %f %f %f\n",v.x(),v.y(),v.z()) ;
	v = v/thickness;
	//fprintf(stderr,"thickness = %f\n",thickness) ;
	//fprintf(stderr,"shell v = %f %f %f\n",v.x(),v.y(),v.z()) ;
	bool outsideShell = testPoint(v) ;
	
	inside = saveInside ;
	outside = saveOutside ;

	return insideShape && outsideShell ;
    }
    else if (!inside && !outside) return false ;
    else if (inside && outside) return true ;
    else
    {
	osg::Vec3d v = xyz * transformationInv ;
	
	return testPoint(v) ;
    }
}
////////////////////////////////////////////////////////////////////////
bool testPoint(osg::Vec3d v)
{
    bool inonly = inside && !outside ;
    
    if (object == CUBE)
    {
	if (v.x() > 1.f || v.x() < -1.f || v.y() > 1.f || v.y() < -1.f || v.z() > 1.f || v.z() < -1.f) return !inonly ;
	else return inonly ;
    }
    else if (object == SPHERE)
    {
	if (v.x()*v.x() + v.y()*v.y() + v.z()*v.z() >1.f) return !inonly ;
	else return inonly ;
    }
    else if (object == TETRAHEDRON)
    {
	if (tetrahedronFaces[0].distance(v)>0.f || 
	    tetrahedronFaces[1].distance(v)>0.f || 
	    tetrahedronFaces[2].distance(v)>0.f || 
	    tetrahedronFaces[3].distance(v)>0.f) return !inonly ;
	else return inonly ;
    }
    else if (object == PYRAMID)
    {
	if (v.z() < -1.f ||
	    pyramidFaces[0].distance(v)>0.f || 
	    pyramidFaces[1].distance(v)>0.f || 
	    pyramidFaces[2].distance(v)>0.f || 
	    pyramidFaces[3].distance(v)>0.f) return !inonly ;
	else return inonly ;
    }
    else return false ;
}

////////////////////////////////////////////////////////////////////////
void make_objectDescription_window()
{
    objectDescription_window = new Fl_Double_Window(300, 300, "shapeDescription") ;
    objectDescription_window->resizable() ;

    objectDescription = new Fl_Multiline_Output(10, 10, 280, 280) ;

}

////////////////////////////////////////////////////////////////////////
void update_objectDescription_window()
{
    char d[4096] ;
    
    osg::Vec3d center(0,0,0) ;
    center = center*transformation ;
    
    if (object == CUBE)
    {
	osg::Vec3d nnn(-1,-1,-1) ;
	nnn = nnn*transformation ;

	osg::Vec3d ppp(1,1,1) ;
	ppp = ppp*transformation ;

	if (shell)
	{
	    osg::Vec3d nnni(-thickness,-thickness,-thickness) ;
	    nnni = nnni*transformation ;
	    
	    osg::Vec3d pppi(thickness,thickness,thickness) ;
	    pppi = pppi*transformation ;

	sprintf(d,
		"%d points selected\n"
		"cube centered at\n"
		"  %6f, %6f, %6f\n"
		"with inside extent from\n"
		"  %6f, %6f, %6f\n"
		"to\n"
		"  %6f, %6f, %6f\n"
		"and outside extent from\n"
		"  %6f, %6f, %6f\n"
		"to\n"
		"  %6f, %6f, %6f\n", 
		numPointsSelected,
		center.x(), center.y(), center.z(), 
		nnni.x(), nnni.y(), nnni.z(), 
		pppi.x(), pppi.y(), pppi.z(),
		nnn.x(), nnn.y(), nnn.z(), 
		ppp.x(), ppp.y(), ppp.z()) ;
	objectDescription->value(d) ;
	}
	else
	{
	sprintf(d,
		"%d points selected\n"
		"cube centered at\n"
		"  %6f, %6f, %6f\n"
		"going from\n"
		"  %6f, %6f, %6f\n"
		"to\n"
		"  %6f, %6f, %6f\n", 
		numPointsSelected,
		center.x(), center.y(), center.z(), 
		nnn.x(), nnn.y(), nnn.z(), 
		ppp.x(), ppp.y(), ppp.z()) ;
	}
	objectDescription->value(d) ;
    }
    else if (object == SPHERE)
    {
	osg::Vec3d xradius(1,0,0) ;
	xradius = (xradius*transformation) - center ;
	osg::Vec3d yradius(0,1,0) ;
	yradius = (yradius*transformation) - center ;
	osg::Vec3d zradius(0,0,1) ;
	zradius = (zradius*transformation) - center ;
	if (shell)
	{
	    sprintf(d,
		    "%d points selected\n"
		    "sphere centered at\n"
		    "  %6f, %6f, %6f\n"
		    "with inner radii in the x, y, z axes\n"
		    "  %6f, %6f, %6f\n" 
		    "and outer radii in the x, y, z axes\n"
		    "  %6f, %6f, %6f\n", 
		    numPointsSelected,
		    center.x(), center.y(), center.z(), 
		    xradius.length()*thickness, yradius.length()*thickness, zradius.length()*thickness,
		    xradius.length(), yradius.length(), zradius.length()) ;
	}
	else
	{
	    sprintf(d,
		    "%d points selected\n"
		    "sphere centered at\n"
		    "  %6f, %6f, %6f\n"
		    "with radii in the x, y, z axes\n"
		    "  %6f, %6f, %6f\n", 
		    numPointsSelected,
		    center.x(), center.y(), center.z(), xradius.length(), yradius.length(), zradius.length()) ;
	}
	objectDescription->value(d) ;
    }
    else if (object == TETRAHEDRON)
    {
	osg::Vec3d verts[4] ;
	verts[0] = tetrahedronVertices[0] * transformation ;
	verts[1] = tetrahedronVertices[1] * transformation ;
	verts[2] = tetrahedronVertices[2] * transformation ;
	verts[3] = tetrahedronVertices[3] * transformation ;

	if (shell)
	{
	    osg::Vec3d vertsi[4] ;
	    vertsi[0] = (tetrahedronVertices[0]*thickness) * transformation ;
	    vertsi[1] = (tetrahedronVertices[1]*thickness) * transformation ;
	    vertsi[2] = (tetrahedronVertices[2]*thickness) * transformation ;
	    vertsi[3] = (tetrahedronVertices[3]*thickness) * transformation ;
	    sprintf(d,
		    "%d points selected\n"
		    "tetrahedron centered at\n"
		    "  %6f, %6f, %6f\n"
		    "with inner peak at\n"
		    "  %6f, %6f, %6f\n"
		    "and inner base at\n"
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "with outer peak at\n"
		    "  %6f, %6f, %6f\n"
		    "and inner base at\n"
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n", 
		    numPointsSelected,
		    center.x(), center.y(), center.z(), 
		    vertsi[0].x(), vertsi[0].y(), vertsi[0].z(), 
		    vertsi[1].x(), vertsi[1].y(), vertsi[1].z(), 
		    vertsi[2].x(), vertsi[2].y(), vertsi[2].z(), 
		    vertsi[3].x(), vertsi[3].y(), vertsi[3].z(),
		    verts[0].x(), verts[0].y(), verts[0].z(), 
		    verts[1].x(), verts[1].y(), verts[1].z(), 
		    verts[2].x(), verts[2].y(), verts[2].z(), 
		    verts[3].x(), verts[3].y(), verts[3].z()) ;
	}
	else
	{
	    sprintf(d,
		    "%d points selected\n"
		    "tetrahedron centered at\n"
		    "  %6f, %6f, %6f\n"
		    "with peak at\n"
		    "  %6f, %6f, %6f\n"
		    "and base at\n"
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n", 
		    numPointsSelected,
		    center.x(), center.y(), center.z(), 
		    verts[0].x(), verts[0].y(), verts[0].z(), 
		    verts[1].x(), verts[1].y(), verts[1].z(), 
		    verts[2].x(), verts[2].y(), verts[2].z(), 
		    verts[3].x(), verts[3].y(), verts[3].z()) ;
	}
	objectDescription->value(d) ;
    }
    else if (object == PYRAMID)
    {
	osg::Vec3d verts[5] ;
	verts[0] = pyramidVertices[0] * transformation ;
	verts[1] = pyramidVertices[1] * transformation ;
	verts[2] = pyramidVertices[2] * transformation ;
	verts[3] = pyramidVertices[3] * transformation ;
	verts[4] = pyramidVertices[4] * transformation ;

	if (shell)
	{
	    osg::Vec3d vertsi[5] ;
	    vertsi[0] = (pyramidVertices[0]*thickness) * transformation ;
	    vertsi[1] = (pyramidVertices[1]*thickness) * transformation ;
	    vertsi[2] = (pyramidVertices[2]*thickness) * transformation ;
	    vertsi[3] = (pyramidVertices[3]*thickness) * transformation ;
	    vertsi[4] = (pyramidVertices[4]*thickness) * transformation ;
	    sprintf(d,
		    "%d points selected\n"
		    "pyramid centered at\n"
		    "  %6f, %6f, %6f\n"
		    "with inner peak at\n"
		    "  %6f, %6f, %6f\n"
		    "and inner base at\n"
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n"
		    "and outer peak at\n"
		    "  %6f, %6f, %6f\n"
		    "and outer base at\n"
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n", 
		    numPointsSelected,
		    center.x(), center.y(), center.z(), 
		    vertsi[0].x(), vertsi[0].y(), vertsi[0].z(), 
		    vertsi[1].x(), vertsi[1].y(), vertsi[1].z(), 
		    vertsi[2].x(), vertsi[2].y(), vertsi[2].z(), 
		    vertsi[3].x(), vertsi[3].y(), vertsi[3].z(), 
		    vertsi[4].x(), vertsi[4].y(), vertsi[4].z(),
		    verts[0].x(), verts[0].y(), verts[0].z(), 
		    verts[1].x(), verts[1].y(), verts[1].z(), 
		    verts[2].x(), verts[2].y(), verts[2].z(), 
		    verts[3].x(), verts[3].y(), verts[3].z(), 
		    verts[4].x(), verts[4].y(), verts[4].z()) ;
	}
	else
	{
	    sprintf(d,
		    "%d points selected\n"
		    "pyramid centered at\n"
		    "  %6f, %6f, %6f\n"
		    "with peak at\n"
		    "  %6f, %6f, %6f\n"
		    "and base at\n"
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n" 
		    "  %6f, %6f, %6f\n", 
		    numPointsSelected,
		    center.x(), center.y(), center.z(), 
		    verts[0].x(), verts[0].y(), verts[0].z(), 
		    verts[1].x(), verts[1].y(), verts[1].z(), 
		    verts[2].x(), verts[2].y(), verts[2].z(), 
		    verts[3].x(), verts[3].y(), verts[3].z(), 
		    verts[4].x(), verts[4].y(), verts[4].z()) ;
	}
	objectDescription->value(d) ;
    }
}

////////////////////////////////////////////////////////////////////////
void writeSelectedData()
{
    if (writeSelectedDataNeeded)
    {
	selectedShm->write(selected) ;
#if 0
	{
	    int c = 0 ;
	    for (int i = 0; i<numPoints; i++) if (selected[i]) c++ ;
	    dtkMsg.add(DTKMSG_WARNING,"hev-shapeSelection: %d selected points\n", c) ;
	}
#endif
	writeSelectedDataNeeded = false ;
    }
    
}

////////////////////////////////////////////////////////////////////////
bool readEligibleData()
{
    static u_int64_t oldWriteCount = 0 ; 
    eligibleShm->read(eligible) ;
    u_int64_t writeCount = eligibleShm->getWriteCount() ;
    if (oldWriteCount !=  writeCount)
    {
	oldWriteCount = writeCount ;
	dtkMsg.add(DTKMSG_INFO, "hev-shapeSelection: eligible write count changed\n") ;
	if  (memcmp(oldEligible, eligible, numPoints))
	{
	    memcpy(oldEligible, eligible, numPoints) ;
	    dtkMsg.add(DTKMSG_INFO, "hev-shapeSelection: eligible points changed\n") ;
#if 0
	    {
		int c = 0 ;
		for (int i = 0; i<numPoints; i++) if (eligible[i]) c++ ;
		dtkMsg.add(DTKMSG_INFO,"hev-shapeSelection: %d eligible points\n", c) ;
	    }
#endif
	    return true ;
	}
    }
    else return false ;
}
