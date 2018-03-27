
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#if 0
#include <X11/Xutil.h>

#include <osgDB/ReadFile>
#endif

#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/ImmersivePane.h>


#if 0
// need to put this last!
#include <X11/extensions/XTest.h>
#endif







////////////////////////////////////////////////////////////////////////
void usage()
    {
    fprintf (stderr,
        "Usage: iris-immersiveWindowBelow immersiveWindowDSOName \n") ;
    dtkMsg.add (DTKMSG_ERROR, 
                "Usage: iris-immersiveWindowsBelow immersiveWindowDSOName \n") ;
    }  // end of usage



static Window 
getWindowByName (Display *dpy, Window parent, const char *windowName)
    {
    char *qName;

    // See if the parent name matches
    XFetchName(dpy, parent, &qName);
    if (qName != NULL)
        {
        if (strcmp (qName, windowName) == 0)
            {
            return parent;
            }
        }


    // If parent doesn't match, check the children
    Window *children, dummy;
    unsigned int nchildren;
    if (!XQueryTree(dpy, parent, &dummy, &dummy, &children, &nchildren))
        {
        return(0);
        }

    Window w = 0;
    for (int i=0; i<nchildren; i++) 
        {
        w = getWindowByName (dpy, children[i], windowName);
        if (w)
            {
            break;
            }
        }

    if (children) XFree ((char *)children);

    return(w);
    } // end of getWindowByName 




static int
setWindowStateBelow (const char *displayName, const char *windowName)
    {


    Display* dpy = XOpenDisplay ( displayName );
    if (dpy == NULL) 
        {
        dtkMsg.add(DTKMSG_ERROR, 
            "iris-immersiveWindowBelow: cannot open X display %s.\n",
            displayName);
        return 1;
        }


#if 0
    Window win;
#else
    Window win = getWindowByName (dpy, XDefaultRootWindow(dpy), windowName);
#endif


    // first we umap the window
    XUnmapWindow (dpy, win);


    // Apparently BELOW is not honored unless it is specified while
    // the window is not mapped.



    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.display = dpy;
    e.xclient.format = 32;
    e.xclient.message_type = XInternAtom (dpy, "_NET_WM_STATE", False);
    e.xclient.window = win;
    e.xclient.data.l[0] = XInternAtom (dpy, "_NET_WM_STATE_ADD", False);
    e.xclient.data.l[1] = XInternAtom (dpy, "_NET_WM_STATE_BELOW", False);
    e.xclient.data.l[2] = 0;
    // fprintf (stderr, "atoms %d %d\n", e.xclient.data.l[0], e.xclient.data.l[1]);
    e.xclient.data.l[3] = 1l;
    e.xclient.data.l[4] = 0l;
    XSendEvent( dpy, DefaultRootWindow(dpy), False, 
                SubstructureRedirectMask|SubstructureNotifyMask, &e );

    // printf (stderr, "JGH just did a change prop on win\n");




// JGH: is the FULLSCREEN CORRECT BELOW????


    Atom states[2];
    states[0] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    states[1] = XInternAtom(dpy, "_NET_WM_STATE_BELOW", False);

    XChangeProperty( dpy, win,
                     XInternAtom(dpy, "_NET_WM_STATE", False),
                     XA_ATOM, 32, PropModeReplace,
                     (unsigned char *) states, 2);


    XFlush(dpy) ;
    XSync (dpy, False) ;





    XSetWindowAttributes attrib;
    attrib.override_redirect = True;
    attrib.override_redirect = False;
    int rtn = 
        XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &attrib);

    // fprintf (stderr, "rtn = %d   BadMatch = %d  BadValue = %d\n", rtn, BadMatch, BadValue);

    XFlush(dpy) ;
    XSync (dpy, False) ;








    // Remap the window.....

    XMapWindow (dpy, win);
    XFlush(dpy) ;
    XSync (dpy, False) ;









    XCloseDisplay (dpy);

    return 0;
    }  // end of setWindowStateBelow

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
    {

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc != 2)
        {
        usage ();
        return -1;
        }

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // you need one of these to read in the DSOs and model files that 
    // define the panes but you don't need a viewer
    iris::SceneGraph isg(false) ;

    iris::LoadFile(argv[1]) ;

    std::vector<osg::ref_ptr<iris::Pane> > pl = iris::Pane::getPaneList() ;

#if 0
    fprintf (stderr, "iris-immersiveWindowBelow size = %d\n", pl.size());
#endif

    for (int i=0; i<pl.size(); i++)
        {
        iris::Pane* pane = pl[i].get() ;
        if (pane->asImmersive())
            {
            const char *winName = pane->getWindow()->getName();
            const char *displayName = 
                    pane->getWindow()->getTraits()->displayName().c_str();

#if 0
            fprintf (stderr, 
                "Found immersive pane named: |%s| on display |%s|\n", 
                winName, displayName);
#endif

            setWindowStateBelow (displayName, winName);
            }  // end of if immersive pane
        }  // end of loop over panes


    return 0 ;

}  // end of main


