#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "idea/Utils.h"



////////////////////////////////////////////////////////////////////////
//
// Set the Size hints so that the window that we're about to unmap
// will come back with same location and size when it is mapped again.
// Probably don't need to do this every time we unmap, but it doesn't
// seem to hurt performance.
static void
userPosSzHints (Display *dpy, Window w)
    {

    // printf ("JGH\n");
    XSizeHints szHints;

    szHints.flags = USPosition | USSize;

    XSetNormalHints (dpy, w, &szHints);
    // XSetWMNormalHints (dpy, w, &szHints);
    // XSetWMSizeHints (dpy, w, &szHints);

    XFlush(dpy) ;
    }  // end of userPosSzHints


////////////////////////////////////////////////////////////////////////

bool idea::UnmapWindowByDisplayName(const char *display, const char *name)
{
  Display *dpy = XOpenDisplay(display) ;
  if (dpy==NULL)
    return false ;

  Window w = idea::GetWindowByName(dpy, name) ;
  if (w==0)
    return false ;

  userPosSzHints (dpy, w);
  XUnmapWindow(dpy,w) ;
  XFlush(dpy) ;

  return true ;
}  // end of idea::UnmapWindowByDisplayName

////////////////////////////////////////////////////////////////////////

bool idea::UnmapAllWindowsByDisplayName(const char *display, const char *name)
{
  Display *dpy = XOpenDisplay(display) ;
  if (dpy==NULL)
    return false ;

  std::vector<Window> wv = idea::GetAllWindowsByName(dpy, name) ;
  if (wv.size()==0)
    return false ;

  for (unsigned int i=0; i<wv.size(); i++)
  {
      userPosSzHints (dpy, wv[i]);
      XUnmapWindow(dpy,wv[i]) ;
      XFlush(dpy) ;
  }

  return true ;
}   // end of idea::UnmapAllWindowsByDisplayName

