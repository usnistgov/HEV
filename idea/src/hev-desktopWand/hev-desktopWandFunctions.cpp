#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Input.H>

#include <dtk.h>
#include <iris.h>

// widgets
Fl_Double_Window *main_window ;
Fl_Group *xyzGroup, *hprGroup ;
Fl_Roller *xRoller, *yRoller, *zRoller, *hRoller, *pRoller, *rRoller ;
Fl_Input *xText, *yText, *zText, *hText, *pText, *rText ;
Fl_Light_Button *moveWithNavigationButton ;
Fl_Button *button0B, *button2B ;
Fl_Light_Button *button0LB, *button2LB ;
Fl_Round_Button *dataButton, *screenButton ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

dtkCoord scene(0.f, 0.f, 0.f, 0.f, 0.f, 0.f) ;
dtkCoord initScene(0.f, 0.f, 0.f, 0.f, 0.f, 0.f) ;
dtkCoord initWorld(0.f, 0.f, 0.f, 0.f, 0.f, 0.f) ;
dtkCoord initXYZHPR(0.f, 0.f, 0.f, 0.f, 0.f, 0.f) ;
float sceneArray[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
float oldSceneArray[6] ={FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
dtkCoord world ;
float worldArray[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
float oldWorldArray[6] ={FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
float step = .002f ;
float oldStep = step ;

// the shared memory that contains the world to Scene transformation
dtkSharedMem *worldToSceneShm = NULL ;
// and the matrix it contains
osg::Matrix worldToSceneMat ;
// and its inverse
osg::Matrix sceneToWorldMat ;

// true if using data (world) coordinate space
// false if using screen coordinate space
bool worldCoords = false ;

// true if wand shm moves with navigation changes
bool moveWithNavigation = false ;

// true if initial data is in world coords
bool initData = false ;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

dtkSharedMem *wandShm = NULL ;
bool wandShmIsCoord = false;
float wand[6] ;
std::string wandShmName = "wand" ;

// shared memory for button and button labels
dtkSharedMem *buttonShm = NULL ;
char button ;
std::string buttonShmName = "buttons" ;

// true if button 0 or button 2 specified, regular
bool button0 = false ;
bool button2 = false ;
// true if button 0 or button 2 specified, light button
bool button0h = false ;
bool button2h = false ;
std::string button0Label, button2Label ;
std::string button0hLabel, button2hLabel ;

unsigned char button0value = 0 ;
unsigned char button2value = 0 ;

// true if only doing xyz
bool xyz = false ;

// true if only doing hpr
bool hpr = false ;

// function templates
bool init(int argc, char **argv) ;
void usage() ;
dtkCoord worldToScene(dtkCoord w) ;
dtkCoord sceneToWorld(dtkCoord d) ;
void reset() ;
void setMoveWithNavigation(bool v) ;
void setWorldCoords(bool v) ;
void initGUI() ;
void updateGUI() ;
void updateShm() ;
void timer_callback(void*) ;
void writeShm() ;
void updateWand(float f, const unsigned int i) ;
void updateX(float f) ;
void updateY(float f) ;
void updateZ(float f) ;
void updateH(float f) ;
void updateP(float f) ;
void updateR(float f) ;
void setButton0(unsigned char b) ;
void setButton2(unsigned char b) ;

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-desktopWand [ --buttons shm ] [ --world ] [ --0 label ] [ --0h label ] [ --2 label ] [ --2h label ] [ --step value ] [--xyz | --hpr ] [ --wand shm ] [ --noescape ] [ x y z [ h p r ] | x y z | h p r ]\n") ;
}

////////////////////////////////////////////////////////////////////////

static dtkSharedMem *
openTransformShm (
  const char *shmName,
  bool mustExist,
  bool createCoord,
  bool & existingIsCoord
  )
        {
        existingIsCoord = true;

        // try to open existing coord file
        dtkSharedMem * shm = NULL;

        shm = new dtkSharedMem (shmName, 0);
        if ( ! shm->isInvalid () )
                {
                int sz = shm->getSize();
                if (sz == 6*sizeof(float))
                        {
                        existingIsCoord = true;
                        return shm;
                        }
                else if (sz == 16*sizeof(double))
                        {
                        existingIsCoord = false;
                        return shm;
                        }
                delete shm;
                }

        // file doesn't exist

        existingIsCoord = false;

        if (mustExist)
                {
                dtkMsg.add(DTKMSG_ERROR,
                        "%s: Unable to open shared memory transformation %s.\n"
                        "     Note that existing files must hold either "
                        " 6 floats (XYZHPR) or 16 doubles (matrix).\n",
                        "hev-desktopWand", shmName);
                return NULL;
                }


        if (createCoord)
                {
                shm = new dtkSharedMem (6*sizeof(float), shmName);
                }
        else
                {
                shm = new dtkSharedMem (16*sizeof(double), shmName);
                }

        if ( shm->isInvalid () )
                {
                dtkMsg.add(DTKMSG_ERROR,
                        "%s: Unable to open shared memory transformation %s.\n"
                        "     Note that existing files must hold either "
                        " 6 floats (XYZHPR) or 16 doubles (matrix).\n",
                        "hev-desktopWand", shmName);
                delete shm;
                shm = NULL;
                }

        return shm;
        } // end of openTransformShm





////////////////////////////////////////////////////////////////////////
bool init(int argc, char **argv)
{

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    int pos ;

    if ((pos=args.findSubstring("--buttons",4))>0 && args.argc()>pos+1)
    {
	args.read(args.argv()[pos], osg::ArgumentParser::Parameter(buttonShmName)) ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: buttons = %s\n",buttonShmName.c_str()) ;
    }

    if ((pos=args.findSubstring("--world",4))>0)
    {
	args.remove(pos) ;
	initData = true ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: initial state is world coordinates\n") ;
    }
    
    if ((pos=args.findSubstring("--noescape",4))>0)
    {
	args.remove(pos) ;
	noEscape = true ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: disabling ESCAPE and window manager close button\n") ;
    }
    
    if ((pos=args.findSubstring("--0"))>0 && args.argc()>pos+1)
    {
	button0 = true ;
	args.read(args.argv()[pos], osg::ArgumentParser::Parameter(button0Label)) ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: button0 label = %s\n",button0Label.c_str()) ;
    }

    if ((pos=args.findSubstring("--0h"))>0 && args.argc()>pos+1)
    {
	button0h = true ;
	args.read(args.argv()[pos], osg::ArgumentParser::Parameter(button0hLabel)) ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: button0h label = %s\n",button0hLabel.c_str()) ;
    }

    if ((pos=args.findSubstring("--2"))>0 && args.argc()>pos+1)
    {
	button2 = true ;
	args.read(args.argv()[pos], osg::ArgumentParser::Parameter(button2Label)) ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: button2 label = %s\n",button2Label.c_str()) ;
    }

    if ((pos=args.findSubstring("--2h"))>0 && args.argc()>pos+1)
    {
	button2h = true ;
	args.read(args.argv()[pos], osg::ArgumentParser::Parameter(button2hLabel)) ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: button2h label = %s\n",button2hLabel.c_str()) ;
    }

    if ((pos=args.findSubstring("--wand",4))>0 && args.argc()>pos+1)
    {
	args.read(args.argv()[pos], osg::ArgumentParser::Parameter(wandShmName)) ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: wand = %s\n",wandShmName.c_str()) ;
    }

    if ((pos=args.findSubstring("--xyz",4))>0)
    {
	args.remove(pos) ;
	xyz = true ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: only xyz\n") ;
    }
    
    if ((pos=args.findSubstring("--hpr",4))>0)
    {
	args.remove(pos) ;
	hpr = true ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: only hpr\n") ;
    }
    
    if ((pos=args.findSubstring("--step",4))>0 && args.argc()>pos+1)
    {
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(step))) return false ;
	oldStep = step ;
	dtkMsg.add(DTKMSG_INFO, "hev-desktopWand: step = %f\n",step) ;
    }

    buttonShm = new dtkSharedMem(sizeof(button), buttonShmName.c_str()) ; 	
    if (buttonShm->isInvalid()) return false ;

#if 0
    wandShm = new dtkSharedMem(sizeof(wand), wandShmName.c_str()) ;
    if (wandShm->isInvalid()) return false ;
#else
    wandShm = openTransformShm ( wandShmName.c_str(), 
                                        false, false, wandShmIsCoord);
    if (wandShm == NULL) return false;

#endif

    if (hpr && xyz)
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-desktopWand: can't turn off both xyz and hpr\n") ;
	return false ;
    }

    // get rid of filename
    args.remove(0) ;

    if (args.argc()!=0 )
    {
	if (args.argc()==3 || args.argc()==6)
	{
	    
	    if (args.argc()==3 && !hpr) // x y z
	    {
		if ( (sscanf(args[0],"%f",&(initXYZHPR.x)) != 1) || 
		     (sscanf(args[1],"%f",&(initXYZHPR.y)) != 1) || 
		     (sscanf(args[2],"%f",&(initXYZHPR.z)) != 1) )
		{
		    return false ;
		}
		dtkMsg.add(DTKMSG_INFO, "x y z = %f %f %f\n",initXYZHPR.x, initXYZHPR.y, initXYZHPR.z) ;
	    }
	    else if (args.argc()==3 && hpr) // h p r
	    {
		if ( (sscanf(args[0],"%f",&(initXYZHPR.h)) != 1) || 
		     (sscanf(args[1],"%f",&(initXYZHPR.p)) != 1) || 
		     (sscanf(args[2],"%f",&(initXYZHPR.r)) != 1) )
		{
		    return false ;
		}
		dtkMsg.add(DTKMSG_INFO, "h p r = %f %f %f\n",initXYZHPR.h, initXYZHPR.p, initXYZHPR.r) ;
	    }
	    else if (args.argc()==6 && !xyz && !hpr) // x y z h p r
	    {
		if ( (sscanf(args[0],"%f",&(initXYZHPR.x)) != 1) || 
		     (sscanf(args[1],"%f",&(initXYZHPR.y)) != 1) || 
		     (sscanf(args[2],"%f",&(initXYZHPR.z)) != 1) ||
		     (sscanf(args[3],"%f",&(initXYZHPR.h)) != 1) || 
		     (sscanf(args[4],"%f",&(initXYZHPR.p)) != 1) || 
		     (sscanf(args[5],"%f",&(initXYZHPR.r)) != 1) )
		{
		    return false ;
		}
		dtkMsg.add(DTKMSG_INFO, "x y z = %f %f %f h p r = %f %f %f\n",initXYZHPR.x, initXYZHPR.y, initXYZHPR.z, initXYZHPR.h, initXYZHPR.p, initXYZHPR.r) ;
	    }
	    else return false ;
	}
	else return false ;
    }

    // irisfly defines this and loads the DSO which updates it
    worldToSceneShm = new dtkSharedMem(sizeof(worldToSceneMat), "idea/worldToScene") ;
    if (worldToSceneShm->isInvalid()) return false ;


    writeShm() ;

    if (initData)  //initXYZHPR and step should be interpreted as world coords
    {
	worldCoords = true ;
	updateShm() ;
	world = initWorld = initXYZHPR ;
	initScene = worldToScene(world) ;

	dtkCoord d0(0,0,0,0,0,0) ;
	dtkCoord w0 = sceneToWorld(d0) ;
	dtkCoord d1(1,0,0,0,0,0) ;
	dtkCoord w1 = sceneToWorld(d1) ;
	oldStep /= sqrt((w1.x-w0.x)*(w1.x-w0.x) + (w1.y-w0.y)*(w1.y-w0.y) + (w1.z-w0.z)*(w1.z-w0.z)) ; 

    }
    else
    {
	initScene = initXYZHPR ;
    }

    scene = initScene ;
    writeShm() ;


    Fl::add_timeout(.05, timer_callback) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
// turn on/off and position window, buttons and rollers
void initGUI()
{
    if (button0Label != "") 
    { 
	button0B->activate() ; 
	button0B->show() ; 
	button0B->label(button0Label.c_str()) ;
    }

    if (button2Label != "") 
    { 
	button2B->activate() ; 
	button2B->show() ; 
	button2B->label(button2Label.c_str()) ;
    }

    if (button0hLabel != "") 
    { 
	button0LB->activate() ; 
	button0LB->show() ; 
	button0LB->label(button0hLabel.c_str()) ;
    }

    if (button2hLabel != "") 
    { 
	button2LB->activate() ; 
	button2LB->show() ; 
	button2LB->label(button2hLabel.c_str()) ;
    }

    if (xyz) hprGroup->hide() ;
    if (hpr) xyzGroup->hide() ;

    short xButton0B, yButton0B, wButton0B, hButton0B ;
    xButton0B = button0B->x() ;
    yButton0B = button0B->y() ;
    wButton0B = button0B->w() ;
    hButton0B = button0B->h() ;
    //printf("button0B: x = %d, y = %d, w = %d, h = %d\n",xButton0B, yButton0B, wButton0B, hButton0B) ;
    
    short xButton2B, yButton2B, wButton2B, hButton2B ;
    xButton2B = button2B->x() ;
    yButton2B = button2B->y() ;
    wButton2B = button2B->w() ;
    hButton2B = button2B->h() ;
    //printf("button2B: x = %d, y = %d, w = %d, h = %d\n",xButton2B, yButton2B, wButton2B, hButton2B) ;
    
    short xButton0LB, yButton0LB, wButton0LB, hButton0LB ;
    xButton0LB = button0LB->x() ;
    yButton0LB = button0LB->y() ;
    wButton0LB = button0LB->w() ;
    hButton0LB = button0LB->h() ;
    //printf("button0LB: x = %d, y = %d, w = %d, h = %d\n",xButton0LB, yButton0LB, wButton0LB, hButton0LB) ;
    
    short xButton2LB, yButton2LB, wButton2LB, hButton2LB ;
    xButton2LB = button2LB->x() ;
    yButton2LB = button2LB->y() ;
    wButton2LB = button2LB->w() ;
    hButton2LB = button2LB->h() ;
    //printf("button2LB: x = %d, y = %d, w = %d, h = %d\n",xButton2LB, yButton2LB, wButton2LB, hButton2LB) ;
    
    short xXYZ, yXYZ, wXYZ, hXYZ ;
    xXYZ = xyzGroup->x() ;
    yXYZ = xyzGroup->y() ;
    wXYZ = xyzGroup->w() ;
    hXYZ = xyzGroup->h() ;
    //printf("xyzGroup: x = %d, y = %d, w = %d, h = %d\n",xXYZ, yXYZ, wXYZ, hXYZ) ;
    
    short xHPR, yHPR, wHPR, hHPR ;
    xHPR = hprGroup->x() ;
    yHPR = hprGroup->y() ;
    wHPR = hprGroup->w() ;
    hHPR = hprGroup->h() ;
    //printf("hprGroup: x = %d, y = %d, w = %d, h = %d\n",xHPR, yHPR, wHPR, hHPR) ;
    
    short xWindow, yWindow, wWindow, hWindow ;
    xWindow = main_window->x() ;
    yWindow = main_window->y() ;
    wWindow = main_window->w() ;
    hWindow = main_window->h() ;
    //printf("main_window: x = %d, y = %d, w = %d, h = %d\n",xWindow, yWindow, wWindow, hWindow) ;
    
    short delta = 0 ;

    if (!button0 && !button2 && !button0h && !button2h) // no buttons
    {
	delta += yXYZ - yButton0B ;
    }

    else if (!button0 && !button2 && (button0h || button2h)) // no click buttons
    {
	delta += yButton0LB - yButton0B ;
	button0LB->position(xButton0LB, yButton0LB-delta) ;
	button2LB->position(xButton2LB, yButton2LB-delta) ;
    }

    else if (!button0h && !button2h && (button0 || button2)) // no press buttons
    {
	delta += yXYZ - yButton0LB ;
    }

    else if ((button0 && button2h && !button0h && !button2) ||
	     (!button0 && !button2h && button0h && button2)) // one click, one press, different buttons
    {
	delta += yButton0LB - yButton0B ;
	button0LB->position(xButton0LB, yButton0LB-delta) ;
	button2LB->position(xButton2LB, yButton2LB-delta) ;
    }

    if (!xyz && !hpr)  // show both xyz & hpr
    {
	xyzGroup->position(xXYZ, yXYZ-delta) ;
	hprGroup->position(xHPR, yHPR-delta) ;
    }

    else if (xyz) // only show xyz, so lop off hpr
    {
	xyzGroup->position(xXYZ, yXYZ-delta) ;
	delta += hHPR ;
    }

    else if (hpr) // only show hpr, so scoot hpr up
    {
	delta += hXYZ ;
	hprGroup->position(xHPR, yHPR-delta) ;
    }

    // shrink window
    main_window->size(wWindow, hWindow-delta) ;
}

////////////////////////////////////////////////////////////////////////
// to get the float to fit in the text box- iris::FloatToString can be too big
std::string FloatToString(const char *fmt, float n)
{
    char s[32] ;
    sprintf(s, fmt, n) ;
    return std::string(s) ;
}

////////////////////////////////////////////////////////////////////////
// update rollers and text output with world or scene value
void updateGUI()
{
    //printf("update GUI\n") ;
    if (worldCoords)
    {
	xRoller->value(world.x) ;
	yRoller->value(world.y) ;
	zRoller->value(world.z) ;
	hRoller->value(world.h) ;
	pRoller->value(world.p) ;
	rRoller->value(world.r) ;
	xText->value(FloatToString("%.4f", world.x).c_str()) ;
	yText->value(FloatToString("%.4f", world.y).c_str()) ;
	zText->value(FloatToString("%.4f", world.z).c_str()) ;
	hText->value(FloatToString("%.2f", world.h).c_str()) ;
	pText->value(FloatToString("%.2f", world.p).c_str()) ;
	rText->value(FloatToString("%.2f", world.r).c_str()) ;
    }
    else
    {
	xRoller->value(scene.x) ;
	yRoller->value(scene.y) ;
	zRoller->value(scene.z) ;
	hRoller->value(scene.h) ;
	pRoller->value(scene.p) ;
	rRoller->value(scene.r) ;
	xText->value(FloatToString("%.4f", scene.x).c_str()) ;
	yText->value(FloatToString("%.4f", scene.y).c_str()) ;
	zText->value(FloatToString("%.4f", scene.z).c_str()) ;
	hText->value(FloatToString("%.2f", scene.h).c_str()) ;
	pText->value(FloatToString("%.2f", scene.p).c_str()) ;
	rText->value(FloatToString("%.2f", scene.r).c_str()) ;
    }
}

////////////////////////////////////////////////////////////////////////
// convert a dtkCoord into a osg::Matrix
osg::Matrix getMatrix(const dtkCoord& c)
{
    osg::Vec3d t(c.x, c.y, c.z) ;
    osg::Quat q = iris::EulerToQuat(c.h, c.p, c.r) ;

    osg::Matrix m ;
    m.makeTranslate(t) ;
    m.preMultRotate(q) ;
    return m ;
}

////////////////////////////////////////////////////////////////////////
// convert a osg::Matrix into a dtkCoord
dtkCoord getCoord(const osg::Matrix& m)
{
    osg::Vec3d t ;
    osg::Quat q  ;
    iris::Decompose(m, &t, &q) ;

    dtkCoord w ;
    w.x = t.x() ; w.y = t.y() ; w.z = t.z() ;
    double h, p, r ;
    iris::QuatToEuler(q, &h, &p, &r) ;
    w.h = h ; w.p = p ; w.r = r ; 
    return w ;
}

////////////////////////////////////////////////////////////////////////
// convert a coordinate in normalized coordinates to world coordinates
// uses matrix in shared memory set up by irisfly
dtkCoord sceneToWorld(dtkCoord d)
{
    //printf("sceneToWorld\n") ;

    osg::Matrix m = getMatrix(d) ;

    // transform by the worldToScene matrix
    m.postMult(sceneToWorldMat) ;
    
    return getCoord(m) ;
}

////////////////////////////////////////////////////////////////////////
// convert a coordinate in world coordinates to normalized coordinates
// uses matrix in shared memory set up by irisfly
dtkCoord worldToScene(dtkCoord w)
{
    osg::Matrix m = getMatrix(w) ;

    // transform by the inverse of the worldToScene matrix
    m.postMult(worldToSceneMat) ;
    
    return getCoord(m) ;
}

////////////////////////////////////////////////////////////////////////
// if a new frame, update other shared memory to allow scene<->world conversions
void updateShm()
{
    // update the transformation matrix
    worldToSceneShm->read(worldToSceneMat.ptr()) ;
    //iris::PrintMatrix(worldToSceneMat) ;
    sceneToWorldMat.invert(worldToSceneMat) ;
}


////////////////////////////////////////////////////////////////////////
// do this every "frame"
void timer_callback(void*)
{

    updateShm() ;

    if (moveWithNavigation)
    {
	// world doesn't change, just scene
	scene = worldToScene(world) ;
	scene.get(sceneArray) ;
	if (memcmp(oldSceneArray, sceneArray, sizeof(sceneArray)))
	{
	    writeShm() ; // only write scene
	    updateGUI() ;
	    memcpy(oldSceneArray, sceneArray, sizeof(sceneArray)) ; 
	}
    }
    else
    {
	// scene doesn't change, just world
	world = sceneToWorld(scene) ;
	world.get(worldArray) ;
	if (memcmp(oldWorldArray, worldArray, sizeof(worldArray)))
	{
	    updateGUI() ;
	    memcpy(oldWorldArray, worldArray, sizeof(worldArray)) ; 
	}
    }

    Fl::repeat_timeout(.05, timer_callback) ;
}

////////////////////////////////////////////////////////////////////////
void setMoveWithNavigation(bool v)
{
    if (v!= moveWithNavigation)
    {
	moveWithNavigation = v ;
    }
    updateGUI() ;
}

////////////////////////////////////////////////////////////////////////
// reset scene to initial value
void reset()
{
    if (initData)
    {
	world = initWorld ;
	scene = worldToScene(world) ;
    }
    else
    {
	scene = initScene ;
	world = sceneToWorld(scene) ;
    }
    updateGUI() ;
    writeShm() ;
}

////////////////////////////////////////////////////////////////////////
void setWorldCoords(bool v)
{
    if (v!= worldCoords)
    {
	worldCoords = v ;
	if (v)
	{
	    dtkCoord d0(0,0,0,0,0,0) ;
	    dtkCoord w0 = sceneToWorld(d0) ;
	    dtkCoord d1(1,0,0,0,0,0) ;
	    dtkCoord w1 = sceneToWorld(d1) ;
	    step *= sqrt((w1.x-w0.x)*(w1.x-w0.x) + (w1.y-w0.y)*(w1.y-w0.y) + (w1.z-w0.z)*(w1.z-w0.z)) ; 
	}
	else
	{
	    step = oldStep ;
	}
	//printf("step = %f\n",step) ;
	xRoller->step(step) ;
	yRoller->step(step) ;
	zRoller->step(step) ;
    }
    updateGUI() ;
}

////////////////////////////////////////////////////////////////////////
static void
coord6ToMat (float coord[6], osg::Matrix & mat)
        {
        mat.makeRotate (iris::EulerToQuat (coord[3], coord[4], coord[5]));
        mat.postMultTranslate (osg::Vec3d (coord[0], coord[1], coord[2]));
        } // coord6ToMat

// write scene 
void writeShm()
{
    scene.get(sceneArray) ;
    //printf("writeShm: scene = %f %f %f  %f %f %f\n",sceneArray[0], sceneArray[1], sceneArray[2], sceneArray[3], sceneArray[4], sceneArray[5]) ;

#if 0
    wandShm->write(sceneArray) ;
#else

    if (wandShmIsCoord)
        {
        wandShm->write(sceneArray) ;
        }
    else
        {
        osg::Matrix mat;
        coord6ToMat (sceneArray, mat);
        wandShm->write (mat.ptr());
        }
        
#endif






}

////////////////////////////////////////////////////////////////////////
void updateWand(float f, const unsigned int i)
{
    if (worldCoords)
    {
	world.d[i] = f ;
	scene = worldToScene(world) ;
    }
    else
    {
	scene.d[i] = f ;
	world = sceneToWorld(scene) ;
    }

    writeShm() ;

}
////////////////////////////////////////////////////////////////////////
void updateX(float f)
{
    updateWand(f,0) ;
}
////////////////////////////////////////////////////////////////////////
void updateY(float f)
{
    updateWand(f,1) ;
}
////////////////////////////////////////////////////////////////////////
void updateZ(float f)
{
    updateWand(f,2) ;
}

////////////////////////////////////////////////////////////////////////
void updateH(float f)
{
    updateWand(f,3) ;
}
////////////////////////////////////////////////////////////////////////
void updateP(float f)
{
    updateWand(f,4) ;
}
////////////////////////////////////////////////////////////////////////
void updateR(float f)
{
    updateWand(f,5) ;
}

////////////////////////////////////////////////////////////////////////
void setButton0(unsigned char b)
{
    if (b == button0value) return ;

    button0value = b ;

    unsigned char old ;
    buttonShm->read(&old) ;
    //printf("\nreading old = %d\n",(int)old) ;
    //printf("b = %d\n",(int)b) ;
    old = old & 0xfe ;
    //printf("masked old = %d\n",(int)old) ;
    old = old | b ;
    //printf("writing old = %d\n",(int)old) ;
    buttonShm->write(&old) ;
}

////////////////////////////////////////////////////////////////////////
void setButton2(unsigned char b)
{
    if (b == button2value) return ;

    button2value = b ;

    unsigned char old ;
    buttonShm->read(&old) ;
    //printf("\nreading old = %d\n",(int)old) ;
    //printf("b = %d\n",(int)b) ;
    old = old & 0xfb ;
    //printf("masked old = %d\n",(int)old) ;
    old = old | b<<2 ;
    //printf("writing old = %d\n",(int)old) ;
    buttonShm->write(&old) ;
}

