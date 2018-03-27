#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <dtk.h>
#include <iris/Utils.h>

char *display ;
Display *dpy;
Window root;
int root_x;
int root_y;

static void signal_catcher(int sig)
{
  // restore the pointer

  if (display) dtkMsg.add(DTKMSG_INFO, "iris-saveRestorePointer: restoring pointer on display %s at %d %d\n", display, root_x, root_y) ;
  else dtkMsg.add(DTKMSG_INFO, "iris-saveRestorePointer: restoring pointer on at %d %d\n", root_x, root_y) ;

  XWarpPointer(dpy, None, root, 0,0, 0,0, root_x, root_y) ;
  XFlush(dpy) ;
}

int main(int argc, char **argv)
{
    
  // send messages to stderr
  dtkMsg.setFile(stderr) ;

  if (argc == 2) display = argv[1] ;
  else if (getenv("DISPLAY")) display = getenv("DISPLAY") ;
  else display = NULL ; 

  dpy = XOpenDisplay(display) ; 

  Window ret_root ;
  Window ret_child ;
  int win_x ;
  int win_y ;
  unsigned int mask ;
  
  root = XDefaultRootWindow(dpy) ;
  
  if(!XQueryPointer(dpy, root, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask)) return 1 ;

  if (display) dtkMsg.add(DTKMSG_INFO, "iris-saveRestorePointer: saving pointer on display %s at %d %d\n", display, root_x, root_y) ;
  else dtkMsg.add(DTKMSG_INFO, "iris-saveRestorePointer: saving pointer on at %d %d\n", root_x, root_y) ;

  // catch signals that kill us off
  iris::Signal(signal_catcher); 

  sleep(100000000) ;

  return 0;
}




