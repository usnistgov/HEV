#include <X11/Xlib.h>

#include "idea/Utils.h"

bool idea::RaiseWindowByDisplayName(const char *display, const char *name)
{
  Display *dpy = XOpenDisplay(display) ;
  if (dpy==NULL)
    return false ;

  Window w = idea::GetWindowByName(dpy, name) ;
  if (w==0)
    return false ;

  XRaiseWindow(dpy,w) ;
  XFlush(dpy) ;
  return true ;

}

////////////////////////////////////////////////////////////////////////

bool idea::RaiseAllWindowsByDisplayName(const char *display, const char *name)
{
  Display *dpy = XOpenDisplay(display) ;
  if (dpy==NULL)
    return false ;

  std::vector<Window> wv = idea::GetAllWindowsByName(dpy, name) ;
  if (wv.size()==0)
    return false ;

  for (unsigned int i=0; i<wv.size(); i++) 
  {
      XRaiseWindow(dpy,wv[i]) ;
      XFlush(dpy) ;
  }
  return true ;

}

