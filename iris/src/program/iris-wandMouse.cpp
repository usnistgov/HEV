#include <X11/Xutil.h>

#include <osgDB/ReadFile>

#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/ImmersivePane.h>

// need to put this last!
#include <X11/extensions/XTest.h>

///////////
//
// iris-wandMouse
//
// The purpose of this program is to transform the position of the 
// wand into a position for the mouse pointer and to take the states
// of the wand buttons and turn these into mouse button states. 
// This is done continuously. The mouse position and button data 
// are used to generate X events.
//
///////////


///////////
// There are four preprocessor symbols that can be used to change how
// this program works:
//      ANG_ONLY           calculate screen coords only on wand angle
//      SCALE_DEFLECTION   scale wand intersection pt relative to screen center
//      PT_AVG_BUF_LEN     length of screen coords averaging buffer
//      GLYPH              position a 3d glyph to track the cursor (obsolete?)
// These are descibed below.



// ANG_ONLY specifies wheter you want to use the angle-only calculation of 
// pixel coords.  The angle-only calculation ignores the wand translation.
// This angle-based calculation does not use SCALE_DEFLECTION.
// Set to  1 if you want the angle-only calculation, which uses only
//                   the orientation of the wand, but not the location
//         0 if you want the screen intersection calculation that is
//                   based on both the location and orientation of the wand.
#ifndef ANG_ONLY
#define ANG_ONLY 1
#endif

// SCALE_DEFLECTION indicates how much to scale the wand intersection point
// relative to the center of the screen. This is used only when ANG_ONLY is
// set to 0.
#ifndef SCALE_DEFLECTION
#define SCALE_DEFLECTION (1.0)
#endif

// PT_AVG_BUF_LEN gives the length of the screen coordinate averaging buffer.
// This must be a positive integer. The higher the number, the greater the
// motion damping.
#ifndef PT_AVG_BUF_LEN
#define PT_AVG_BUF_LEN (4)
#endif

// GLYPH specifies whether you want to have a 3D glyph in the scenegraph to 
// track the cursor RATHER THAN moving the X pointer.  The movement of the
// 3D glyph is done via control messages to stdout.
// Set to  1 if you want the glyph rather than the X pointer
//         0 if you do not want the glyph and you want the X pointer
#ifndef GLYPH
#define GLYPH 0
#endif
// JGH: 1/16/2017  I think that we should completely remove the GLYPH option.
//      It seems to me that the GLYPH option (setting GLYPH to 1) would render
//      iris-wandMouse useless in the current HEV environment. This is because
//      the GLYPH option does not send the X events to properly position
//      the X pointer. This position is needed by X to correctly interact
//      with the X-based menus that HEV uses. Even if we changed to code
//      to do both the 3D GLYPH positioning and the X operations, I don't
//      see what additional functionality the GLYPH option provides.

//
///////////

#ifndef VERBOSE
#define VERBOSE 0
#endif


// how small can it get before we toss it?
const float SMALLEST_T = -10.f ;

bool running = true ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    //fprintf (stderr,  "iris-wandMouse, PID %d, "
    //                  "caught signal %d, starting exit sequence ...\n", 
    //                    getpid(), sig);

#if GLYPH
    printf("UNLOAD iris-wandMouseCursorDCS\nUNLOAD iris-wandMouseCursor\n") ;
    fflush(stdout) ;
#endif
    running = false ;
}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr, "Usage: iris-wandButton [ --help ] "
                    "[ --button N buttonShm ] "
                    "[ --usleep t ] [ --wand wandShm ] [ file ... ] \n") ;
}

// button shared memory
struct Button
{
    std::string shmName ;
    dtkSharedMem* shm ;
    int mask ;
    unsigned char value ;
    int num ;
} ;

std::vector<Button> buttons;

////////////////////////////////////////////////////////////////////////
bool addButton(std::string shmName, int mask, int num)
{
    Button* b = new Button ;
    b->shmName = shmName ;
    b->mask = mask ;
    b->num = num ;
    b->shm = new dtkSharedMem(sizeof(b->value), shmName.c_str()) ;
    if (!(b->shm) || b->shm->isInvalid())
    {
        dtkMsg.add(DTKMSG_ERROR, 
            "iris-wandMouse: can't open button shared memory \"%s\"\n", 
            b->shmName.c_str()) ;
        return false ;
    }
    b->shm->read(&(b->value)) ;
    buttons.push_back(*b) ;
    return true ;
}
////////////////////////////////////////////////////////////////////////
static void
averagePtBuf (double ptBuf[][2], int bufLen, double avgPt[2])
{
    avgPt[0] = avgPt[1] = 0;

    for (int i = 0; i < bufLen; i++)
    {
        avgPt[0] += ptBuf[i][0];
        avgPt[1] += ptBuf[i][1];
    }

    avgPt[0] /= bufLen;
    avgPt[1] /= bufLen;

    return;
} // end of averagePtBuf


int main(int argc, char **argv)
{


#if VERBOSE
    fprintf (stderr, "PT_AVG_BUF_LEN = %d\n", PT_AVG_BUF_LEN);
    fprintf (stderr, "SCALE_DEFLECTION = %g\n", SCALE_DEFLECTION);
    if (ANG_ONLY)
    {
        fprintf (stderr, "Using angle calculation\n");
    }
    else
    {
        fprintf (stderr, "Using pointing calculation\n");
    }
#endif

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // get data from arguments
    int pos ;

    if ((pos=args.findSubstring("--help",4))>0)
    {
        usage() ;
        return 0 ;
    }

    int ticks = iris::GetUsleep() ;
    if ((pos=args.findSubstring("--usleep",4))>0) 
            args.read(args.argv()[pos], osg::ArgumentParser::Parameter(ticks)) ;
    // fprintf(stderr, "ticks = %d\n",ticks) ;

    // wand shared memory
    std::string wandShmName = "wandMatrix" ;
    if ((pos=args.findSubstring("--wand",4))>0) 
      args.read(args.argv()[pos], osg::ArgumentParser::Parameter(wandShmName)) ;
    // fprintf(stderr, "wandShmName = %s\n",wandShmName.c_str()) ;
    dtkSharedMem* wandShm ;
    osg::Matrix wand ;
    osg::Matrix oldWand ;
    wandShm = new dtkSharedMem(sizeof(double)*16, wandShmName.c_str()) ;
    if (!wandShm || wandShm->isInvalid())
    {
        dtkMsg.add(DTKMSG_ERROR, 
            "iris-wandMouse: can't open shared memory \"%s\"\n", 
            wandShmName.c_str()) ;
        return 1 ;
    }

    // buttons
    while ((pos=args.findSubstring("--button",4))>0)
    {
        std::string shmName ;
        int mask ;
        int num ;
        if (!args.read( args.argv()[pos], 
                        osg::ArgumentParser::Parameter(shmName), 
                        osg::ArgumentParser::Parameter(mask), 
                        osg::ArgumentParser::Parameter(num)))
        {
            dtkMsg.add(DTKMSG_ERROR, 
                        "iris-wandMouse: error parsing button parameters\n") ;
            return 1 ;
        }

        if (!addButton(shmName, mask, num)) return 1 ;
    }

    if (buttons.size() == 0)
    {
        if (!addButton("buttons", 2, 1)) return 1 ;
    }

    // you need one of these to read in the DSOs and model files that 
    // define the panes but you don't need a viewer
    iris::SceneGraph isg(false) ;

    // load DSOs
    iris::LoadFile(&args) ;
    
    // find the first immersive pane
    // this is only done once- if you think you might be moving your
    // immersive pane/window/display when running, you need to put this in
    // the loop
    iris::ImmersivePane* immersivePane = NULL ;
    
    std::vector<osg::ref_ptr<iris::Pane> > pl = iris::Pane::getPaneList() ;
    for (int i=0; i<pl.size() && immersivePane == NULL; i++)
    {
        iris::Pane* pane = pl[i].get() ;
        if (pane->asImmersive())
        {
            if (!immersivePane) 
            {
                immersivePane = pane->asImmersive() ;
            }
            else
            {
                dtkMsg.add(DTKMSG_WARNING,
                    "iris-wandMouse: more than one immersive pane specified, "
                    "using the first one encountered\n") ;
            }
        }
    }

    if (immersivePane == NULL)
    {
        dtkMsg.add(DTKMSG_ERROR,"iris-wandMouse: no immersive pane found\n") ;
        return 1 ;
    }

    // collect data about the pane, window and display
    struct 
    {
        osg::Vec2 extent ;
        osg::Vec3 center ;
        osg::Quat orientation ;
        int x ;
        int y ;
        int width ;
        int height ;
    } pane ;
    pane.extent = immersivePane->getExtent() ;
    pane.center = immersivePane->getCenter() ;
    pane.orientation = immersivePane->getOrientation() ;
    immersivePane->getViewport(&pane.x, &pane.y, &pane.width, &pane.height) ; 

    iris::Window* const immersiveWindow = immersivePane->getWindow() ;
    osg::GraphicsContext::Traits* traits = immersiveWindow->getTraits() ;
    struct
    {
        int x ;
        int y ;
        unsigned int width ;
        unsigned int height ;
        const char* name ;
    } window ;
    window.x = traits->x ;
    window.y = traits->y ;
    window.width = traits->width ;
    window.height = traits->height ;
    window.name = immersiveWindow->getName() ;
    if (traits->windowDecoration) 
    {        
        dtkMsg.add(DTKMSG_WARNING, 
            "iris-wandMouse: immersive pane's window has decorations; "
            "this might affect the proper positioning of the X cursor\n") ;
    }

    struct
    {
        std::string name ;
        unsigned int width ;
        unsigned int height ;
    } display ;
    display.name = traits->displayName() ;
    osg::GraphicsContext* gc = immersiveWindow->getGraphicsContext() ;
    osg::GraphicsContext::WindowingSystemInterface* wsi = 
                                gc->getWindowingSystemInterface();
    wsi->getScreenResolution (
        osg::GraphicsContext::ScreenIdentifier(traits->hostName, 
                                               traits->displayNum, 
                                               traits->screenNum), 
        display.width, display.height );

#if VERBOSE
    fprintf(stderr,"DISPLAY = %s\n",display.name.c_str()) ;
    fprintf(stderr," display width, height = %d, %d\n", 
                            display.width, display.height);
    fprintf(stderr," window: %s\n",immersiveWindow->getName()) ;
    fprintf(stderr,"    x,y= %d,%d  width,height =%d,%d\n",
                            window.x, window.y, window.width, window.height) ;
    fprintf(stderr,"    pane: extent = %f %f\n",
                            pane.extent.x(),pane.extent.y()) ;
    fprintf(stderr,"         center = %f %f %f\n",
                            pane.center.x(),pane.center.y(),pane.center.z()) ;
    fprintf(stderr,"         orientation (x,y,z,w) = %f, %f %f %f\n",
                            pane.orientation.x(),
                            pane.orientation.y(),
                            pane.orientation.z(),
                            pane.orientation.w()) ;
    fprintf(stderr,"         viewport (x,y,w,h) = %d, %d %d %d\n", 
                            pane.x, pane.y, pane.width, pane.height) ;

    fflush (stderr);
#endif

    // get hooks into X
    Display* Xdisplay;
    Window root;
    if ((Xdisplay = XOpenDisplay(display.name.c_str())) == NULL) {
        dtkMsg.add(DTKMSG_ERROR, "iris-wandMouse: cannot open X display %s.\n",
                        display.name.c_str());
        return 1;
    }
    root = DefaultRootWindow(Xdisplay);
    
    int ev, er, ma, mi;
    if(!XTestQueryExtension(Xdisplay, &ev, &er, &ma, &mi))
    {
        dtkMsg.add(DTKMSG_ERROR, 
            "iris-wandMouse: XTest extension not supported on server.\n");
        return 1;
    }

#if GLYPH
    // put the glyph into the scene graph
    printf("QDCS iris-wandMouseCursorDCS\n"
           "ADDCHILD iris-wandMouseCursorDCS scene\n"
           "LOAD iris-wandMouseCursor cursor.osg\n"
           "ADDCHILD iris-wandMouseCursor iris-wandMouseCursorDCS\n") ;
    fflush(stdout) ;
#endif

    // catch signals that kill us off
    iris::Signal(signal_catcher); 

    //////
    // setup circular buffer of 2D pts
    int currBufPt = 0;
    double ptBuf[PT_AVG_BUF_LEN][2];
    for (int i = 0; i < PT_AVG_BUF_LEN; i++)
    {
        ptBuf[i][0] = ptBuf[i][1] = 0;
    }

    //////


    // read wand and buttons in a loop
    while (running)
    {


        ////// First we calculate a new position for the pointer / glyph

        wandShm->read(wand.ptr()) ;
        if (oldWand != wand)   // did wand position change?
        {
            oldWand = wand ;

            // JGH: the following variable is currently (11/2016) only
            // being used as a flag. If it is < FLT_MAX, then we have
            // found a new mouse position based on the wand and we need
            // to move the cursor.
            float distance2FromCursorToCenterOfViewport = FLT_MAX ;
    

#if GLYPH
            osg::Vec3 closest3Dpt ;
#endif
            osg::Vec2 closest2Dpt ;

            bool insideWindow = false ;

            // figure out line from wand
            // parametric line is pt0 + t*pt
            osg::Vec3 pt0 = wand.getTrans() ;
            osg::Quat wandRot = wand.getRotate() ;
            // pt is 0,1,0 rotated by wand- 0,1,0 is p1 with wand 
            // pointing straight ahead, no rotation
            osg::Vec3 pt = wandRot * osg::Vec3(0, 1, 0) ;

#if VERBOSE
            fprintf(stderr,"wand trans: pt0: %+3.3f %+3.3f %+3.3f\n", 
                            pt0.x(), pt0.y(), pt0.z()) ;
            fprintf(stderr,"wand vec:   pt:  %+3.3f %+3.3f %+3.3f\n", 
                            pt.x(), pt.y(), pt.z()) ;

            fprintf (stderr, "X degree deflection: %g\n",
                        (atan (pt.x()/pt.y()) * (180.0/M_PI)) );
            fprintf (stderr, "Z degree deflection: %g\n",
                        (atan (pt.z()/pt.y()) * (180.0/M_PI)) );
#endif


            if (ANG_ONLY)
            {

                // Get the angle of deflection of the wand relative
                // to the X and Y directions of the screen.
                double pX = atan2 (pt.x(), pt.y()) * (180.0/M_PI) + 60;
                double pY = atan2 (pt.z(), pt.y()) * (180.0/M_PI) + 45;
                // Scale and clamp these deflections to range [0,1]
                pX /= 120.0;
                pY /= 90.0;
                pX = std::max (std::min (pX, 1.0), 0.0);
                pY = std::max (std::min (pY, 1.0), 0.0);

                // Use these to generate pixel coordinates.
                // Should we make these floating point coords?
                int iX = (window.x + pane.x) + (pane.width * pX);
                int iY = (window.y + window.height - pane.y) - 
                                                 (pane.height * pY);
#if VERBOSE
                fprintf (stderr, "ANG_ONLY screen pt:  iX %d   iY %d\n", iX, iY);
#endif
                ptBuf[currBufPt][0] = iX;
                ptBuf[currBufPt][1] = iY;
                currBufPt = (currBufPt+1) % PT_AVG_BUF_LEN;

                // In the current implementation, the angular deflection is linearly mapped
                // to screen movement. It might be interesting to apply a function so that
                // it requires greater deflection near the edges of the screen to achieve
                // the same screen movement that you get near the center of the screen.
                // The rationale for this is that menus are typically positioned near the
                // edges of the screen and interaction with menus requires greater wand 
                // pointer positional accuracy.

                // The following variable is not really used except as a flag.
                // Set to zero to trigger cursor movement.
                distance2FromCursorToCenterOfViewport = 0.0; 

                // end of if ANG_ONLY
            }  
            else
            {
                // if not ANG_ONLY

                // In this section we do the screen intersection calculation
                // that is based on both the location and orientation of 
                // the wand. This is the method that was originally 
                // implemented for cursor positioning.

                // get parametric form of plane- Ax +By +Cz +D = 0
                // for A,B,C, calculate xyz normal 
                // rotate xyz = 0,1,0 through hpr of screen

                osg::Vec3 ABC = pane.orientation * osg::Vec3(0, 1, 0) ;
                // D is -distance
                float D = -(ABC * pane.center) ;

                float denom = ABC * pt ;
                if (denom!=0) // wand vector is parallel to plane if denom == 0
                {
                    float t = ((ABC*pt0) + D)/denom ;
                    if (t<0.f && t>SMALLEST_T) //wand intersects plane of screen
                    {
                        // intersection point - we subtract since plane is 
                        // pointing the wrong way
                        osg::Vec3 ipt = pt0 - pt*t ;
                        // fprintf(stderr, "intersection point: %f %f %f\n",
                        //            ipt.x(),ipt.y(),ipt.z()) ;
#if GLYPH
                        // ipt gets stepped on in next set of transformations
                        osg::Vec3 saveIpt(ipt) ;
#endif
                        // unrotate and translate the intersection point so 
                        // we can compare with size of screen
                        osg::Matrix rotMat ;
                        rotMat.setRotate(pane.orientation) ;
                        rotMat.setTrans(pane.center) ;
                    
                        osg::Matrix rotMatInv ;
                        rotMatInv.invert(rotMat) ;
                    
                        osg::Matrix mat ;
                        mat.setTrans(ipt) ;
                        mat.postMult(rotMatInv) ;
                        ipt = mat.getTrans() ;

                        // ipt is now the intersection point

                        float width2 = pane.extent.x()/2 ;
                        float height2 = pane.extent.y()/2 ;

                        // JGH: It is unclear how newDist2 is meaningfully used.
                        //      At this point, as long as it is calculated to
                        //      be less than FLT_MAX, it triggers the movement
                        //      of the mouse pointer.
                        float newDist2 ;
                    
                        //fprintf(stderr,"ipt.x() = %f, width2 = %f\n",
                        //                  ipt.x(),width2) ;
                        //fprintf(stderr,"ipt.z() = %f, height2 = %f\n",
                        //                  ipt.z(),height2) ; 

                        // scale the intersection point about the origin
                        ipt.x() *= SCALE_DEFLECTION;
                        ipt.z() *= SCALE_DEFLECTION;
                        // could we say ipt *= SCALE_DEFLECTION; ?

                        // are we intersecting inside the pane? 
                        if (ipt.x() <  width2  && 
                            ipt.x() > -width2  &&
                            ipt.z() <  height2 && 
                            ipt.z() > -height2    )
                        {
                            // yes, we are inside the pane
                            newDist2 = 0.f ;
                            insideWindow = true ;
                        }
                        else
                        {
                            // We are not inside the pane, so we
                            // need to modify ipt and saveIpt so they're on the 
                            // edge of the window

                            //fprintf(stderr,"edge!\n") ;
                            newDist2 = ipt*ipt ;
                        
                            // clamp ipt, 3d positionn XZ plane, to screen width
                            if (ipt.x() > width2) ipt.x() = width2 ;
                            else if (ipt.x() < -width2) ipt.x() = -width2 ;

                            if (ipt.z() > height2) ipt.z() = height2 ;
                            else if (ipt.z() < -height2) ipt.z() = -height2 ;

                            //fprintf(stderr,"new ipt.x() = %f, ipt.z() = %f\n",
                            //               ipt.x(), ipt.z()) ;

#if GLYPH
                            // need to unrotate and untranslate to get back to 
                            // world 3d position
                            osg::Matrix newIptMat ;
                            newIptMat.setTrans(ipt) ;
                            newIptMat.postMult(rotMat) ;
                            saveIpt = newIptMat.getTrans() ;
#endif
                        } // if not inside a pane, so move ipt to edge

                        // move cursor if we have a valid intersection pt
                        if (newDist2 < distance2FromCursorToCenterOfViewport) 
                        {
                            distance2FromCursorToCenterOfViewport = newDist2 ; 
#if GLYPH
                            closest3Dpt = saveIpt ;
#endif                                            
                            // to move cursor you need window pixel coordinates!
                            closest2Dpt.x() = 
                                (window.x + pane.x) + 
                              (pane.width / pane.extent.x()) * (ipt.x()+width2);

                            closest2Dpt.y() = 
                                (window.y + window.height - pane.y) -
                            (pane.height / pane.extent.y()) * (ipt.z()+height2);

#if VERBOSE
                            fprintf (stderr, "Screen intersection:   x %g   y %g\n", 
                                        (double)closest2Dpt.x(), (double)closest2Dpt.y());
#endif
                            ptBuf[currBufPt][0] = closest2Dpt.x();
                            ptBuf[currBufPt][1] = closest2Dpt.y();
                            currBufPt = (currBufPt+1) % PT_AVG_BUF_LEN;

                        } // if we have a valid (possibly modified) 
                          // intersection pt

                    } // if wand intersects plane of screen

                } // wand not pointing parallel to screen; if (denom!=0)

            } // end of if  not ANG_ONLY

            
            // Do we have a valid point to which to move the cursor?
            if (distance2FromCursorToCenterOfViewport < FLT_MAX)
            {
                // We either move the wand mouse glyph or use XWarpPointer to 
                // move the X pointer, but not both.
#if GLYPH
                // move the glyph
                printf("QDCS iris-wandMouseCursorDCS %f %f %f %f %f %f %f\n",
                        closest3Dpt.x(),closest3Dpt.y(),closest3Dpt.z(), 
                        pane.orientation.x(), 
                        pane.orientation.y(), 
                        pane.orientation.z(), 
                        pane.orientation.w()) ;
                fflush(stdout) ;

#else
                // move the X pointer

                // average the buffer then move the pointer
                double avgPt[2];
                averagePtBuf (ptBuf, PT_AVG_BUF_LEN, avgPt);
                XWarpPointer ( Xdisplay, None, root, 0, 0, 0, 0, 
                               int(avgPt[0]), int(avgPt[1]) );

                XFlush(Xdisplay);
                XSync(Xdisplay,0);
#endif                
            } // if we calculated a valid distance2FromCursorToCenterOfViewport

        } // if wand position changed


        //////  We are done with the pointer/glyph. Now we do the buttons.


        // Was a button pressed? 
        // Loop through each button.
        for (unsigned int i=0; i<buttons.size(); i++)
        {
            unsigned char value ;
            buttons[i].shm->read(&value) ;
            if ( (buttons[i].value & buttons[i].mask) != 
                            (value & buttons[i].mask))
            {
                buttons[i].value = value ;
                // fprintf(stderr,"button %d: %d mask: %d\n",
                //     buttons[i].num,value,buttons[i].mask) ;
                
                ::Window thisWindow = 
                    iris::GetWindowByName(Xdisplay, window.name) ; 
                if (thisWindow == 0)
                {
                    dtkMsg.add(DTKMSG_ERROR, 
                     "iris-wandMouse: can't get windowID of immersive pane.\n");
                    return 1 ;
                }
                //fprintf(stderr, "this window = 0x%x\n",thisWindow) ;
                
                ::Window focusedWindow ;
                int rev ;
                XGetInputFocus(Xdisplay, &focusedWindow, &rev) ;
                if (focusedWindow == 0)
                {
                    dtkMsg.add(DTKMSG_ERROR, 
                     "iris-wandMouse: can't get windowID of focused window.\n");
                    return 1 ;
                }

                // fprintf(stderr,"focusedWindow = 0x%x\n",focusedWindow) ;

#if 0
                // JGH: I have removed the following test that decides 
                //      whether or not to send a button press.  
                //      I believe that this was originally done to prevent 
                //      the immersive window from being raised.  
                //      But we now (4/29/2013) have a scheme that prevents 
                //      the immersive window from being raised.  So we will 
                //      always send the button press.
                //
                // JGH: 1/13/2017  Is it possible that the removal of this
                //      test has caused the the problem that we sometimes see 
                //      in which the cursor (as controlled by iris-wandMouse) 
                //      stops moving and we can free it up by allowing the real
                //      mouse to move the cursor or to push a button? 
                //      Perhaps sending a button press to an unintended window 
                //      causes some screw-up with focus or some other X weirdness?
                //
                //      By the way, it might be interesting to "debounce" the
                //      button release. It seems to me that the current wand has
                //      stiffer buttons and when you're trying to hold down a button,
                //      there is a slight tendency to have an unintended button release 
                //      followed immediately by a press. It would be easy to filter 
                //      out such short button releases.
        
                // only send a button press if the cursor is not in 
                // the immersive pane
                if (thisWindow != focusedWindow)
#endif
                {
                    bool state ;
                    if (value)
                    {
                        state = true ;
                        dtkMsg.add(DTKMSG_INFO, 
                            "iris-wandMouse: pressing X button %d\n", 
                            buttons[i].num) ;
                    }
                    else
                    {
                        state = false ;
                        dtkMsg.add(DTKMSG_INFO, 
                            "iris-wandMouse: releasing X button %d\n", 
                            buttons[i].num) ;
                    }
                    if(!XTestFakeButtonEvent(Xdisplay, buttons[i].num, 
                                                          state, CurrentTime))
                    {
                        dtkMsg.add(DTKMSG_ERROR, 
                          "iris-wandMouse: XTestFakeButtonEvent failed.\n");
                        return 1 ;
                    }
                    XFlush(Xdisplay);
                    XSync(Xdisplay,0);
                }
#if 0
                else
                {
                    dtkMsg.add(DTKMSG_INFO, 
                      "iris-wandMouse:  "
                      "ignoring button event in immersive pane\n") ; 
                }
#endif


            }
        } // loop over buttons

        // Done with the buttons

        usleep(ticks) ;  // wait a little bit....

    } // while (running)

    return 0 ;

} // end of main


