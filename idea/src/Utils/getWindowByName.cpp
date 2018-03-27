#include <X11/Xlib.h>
#include <string.h>

#include "idea/Utils.h"

// taken from code in dsimple.c

static Window GetWindowByNameTop(Display *dpy, const Window top, const char *name)
{
    Window *children, dummy;
    unsigned int nchildren;
    int i;
    Window w=0;
    char *window_name;
  
    if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
	return(top);
  
    if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
	return(0);
  
    for (i=0; i<nchildren; i++) {
	w = GetWindowByNameTop(dpy, children[i], name);
	if (w)
	    break;
    }
    if (children) XFree ((char *)children);
    return(w);
}

Window idea::GetWindowByName(Display *dpy, const char *name)
{
    return GetWindowByNameTop(dpy, XDefaultRootWindow(dpy), name) ;
}

////////////////////////////////////////////////////////////////////////
static std::vector<Window> GetAllWindowsByNameTop(Display *dpy, const Window top, const char *name)
{
    Window *children, dummy;
    unsigned int nchildren;
    int i;
    std::vector<Window> wv ;
    char *window_name;
  
    if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
    {
	wv.push_back(top) ;
	return(wv);
    }
  
    if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
    {
	return(wv) ;
    }
  
    for (i=0; i<nchildren; i++) {
	Window w = GetWindowByNameTop(dpy, children[i], name);
	if (w) wv.push_back(w) ;
    }
    if (children) XFree ((char *)children);
    return(wv);
}

std::vector<Window> idea::GetAllWindowsByName(Display *dpy, const char *name)
{
    return GetAllWindowsByNameTop(dpy, XDefaultRootWindow(dpy), name) ;
}

