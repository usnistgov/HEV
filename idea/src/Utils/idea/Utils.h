#ifndef _IDEAUTILS_H_
#define _IDEAUTILS_H_

#include <X11/Xlib.h>
#include <string>
#include <vector>

namespace idea
{

  /* given a display and window name, return the Window, or 0 is no window
     is found
   */
  ::Window GetWindowByName(::Display *dpy, const char *name) ;

  /* map an X11 window by display and name
   *
   * returns true if window is found
   */

  bool MapWindowByDisplayName(const char *display, const char *name) ;

  /* unmap an X11 window by display and name
   *
   * returns true if window is found
   */

  bool UnmapWindowByDisplayName(const char *display, const char *name) ;

  /* raise an X11 window by display and name
   *
   * returns true if window is found
   */

  bool RaiseWindowByDisplayName(const char *display, const char *name) ;

  /* lower an X11 window by display and name
   *
   * returns true if window is found
   */

  bool LowerWindowByDisplayName(const char *display, const char *name) ;

  ////////////////////////////////////////////////////////////////////////

  /* given a display and window name, return all of the Windows, or 0 is no window
     is found
   */
  std::vector< ::Window > GetAllWindowsByName(::Display *dpy, const char *name) ;

  /* map all X11 windows by display and name
   *
   * returns true if a window is found
   */

  bool MapAllWindowsByDisplayName(const char *display, const char *name) ;

  /* unmap all X11 windows by display and name
   *
   * returns true if a window is found
   */

  bool UnmapAllWindowsByDisplayName(const char *display, const char *name) ;

  /* raise all X11 windows by display and name
   *
   * returns true a window is found
   */

  bool RaiseAllWindowsByDisplayName(const char *display, const char *name) ;

  /* lower aall X11 windows by display and name
   *
   * returns true a window is found
   */

  bool LowerAllWindowsByDisplayName(const char *display, const char *name) ;

  /* Parse fltk-specific command line options.
   *
   * returns malloc()ed memory that is pointers to
   * the strings that are passed in const char** argv.  You may use
   * free() on the returned values *fltk_argv and *other_argv.  argv[0]
   * is included in both returned argument arrays.  If any handles
   * or pointers are NULL false is returned.
   */

  bool fltkOptions(const int argc, const char **argv,
                   int *fltk_argc, char ***fltk_argv,
                   int *other_argc, char ***other_argv) ;

  /* looks for the supplied string in the keyword string, CaSeInSenSiTiVe.  
   *
   * returns true if supplied matchs the first minLength characters of
   * keyword, or all of the characters of supplied match keyword is minLength
   * is zero or omitted.
   */

  bool isSubstring(const std::string keyword, const std::string supplied,
                   const unsigned int minLength=0) ;

  /* puts the argv arguments into a std::vector.
   *
   * returns a vector of argv
   */
  std::vector<std::string> argvArgcToStringVector(const int argc,
                                                  const char **argv) ;

  /** read a glob pattern from a directory.
   *
   * dir is the directory to read.
   * pat is the glob pattern to use.
   * returns a vector of strings of the filenames matching pat in dir.
   */
  std::vector<std::string> globDir(char const* dir, char const* pat) ;
}

#endif
